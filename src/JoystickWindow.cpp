#include <chrono>
#include <cstring>
#include <iostream>
#include <ranges>
#include <string>

#include <SDL_timer.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <implot.h>

#include <sdl2xx/clipboard.hpp>
#include <sdl2xx/game_controller.hpp>

#include "JoystickWindow.hpp"

#include "JoystickListWindow.hpp"


using namespace std::literals;

using std::cout;
using std::endl;


namespace {

    const unsigned max_history_size = 60 * 5;

    const ImVec4 key_color = {1.0, 1.0, 0.5, 1.0};


    sdl::vec2
    to_pos(sdl::joystick::hat_dir dir)
    {
        using sdl::joystick::hat_dir;
        switch (dir) {
            case hat_dir::centered:
                return {0, 0};
            case hat_dir::down:
                return {0, -1};
            case hat_dir::down_left:
                return {-1, -1};
            case hat_dir::down_right:
                return {1, -1};
            case hat_dir::left:
                return {-1, 0};
            case hat_dir::left_up:
                return {-1, 1};
            case hat_dir::right:
                return {1, 0};
            case hat_dir::right_up:
                return {1, 1};
            case hat_dir::up:
                return {0, 1};
            default:
                return {2, 2};
        }
    }


    std::vector<std::string>
    split(const std::string& input,
          char separator)
    {
        using std::string;
        std::vector<string> result;

        string::size_type start, finish;
        start = 0;
        while ((finish = input.find(separator, start)) != string::npos) {
            result.push_back(input.substr(start, finish - start));
            start = finish + 1; // skip over separator
        }
        result.push_back(input.substr(start));
        return result;
    }


    std::map<std::string, std::string>
    parse_mapping(const std::string& input)
    {
        std::map<std::string, std::string> mapping;

        auto fields = split(input, ',');
        if (fields.size() < 2)
            throw std::runtime_error{"invalid mapping string"};
        mapping["guid"] = fields[0];
        mapping["name"] = fields[1];
        for (const auto& token : fields | std::views::drop(2)) {
            if (token.empty())
                continue;
            // cout << "splitting token \"" << token << "\"" << endl;
            auto keyval = split(token, ':');
            if (keyval.size() != 2) {
                cout << "Error parsing mapping string: \"" << input << "\"" << endl;
                continue;
            }
            mapping[keyval.front()] = keyval.back();
        }
        return mapping;
    }


    [[maybe_unused]]
    void
    dump_mapping(const std::map<std::string, std::string>& mapping)
    {
        cout << "Mapping:\n";
        for (auto& [key, val] : mapping) {
            cout << "    " << key << " : " << val << "\n";
        }
        cout << endl;
    }


    std::string
    build_mapping(const std::map<std::string, std::string>& mapping)
    {
        std::string result = mapping.at("guid") + ",";
        result += mapping.at("name") + ",";

        for (auto& [key, val] : mapping) {
            if (key == "guid" || key == "name")
                continue;
            if (val.empty())
                continue;
            result += key + ":" + val + ",";
        }

        return result;
    }

} // namespace


JoystickWindow::JoystickWindow(JoystickListWindow* parent,
                               sdl::joystick::instance_id id) :
    parent{parent},
    id{id},
    dev{sdl::joystick::device::from_id(id)}
{
    axis_histories.resize(dev.get_num_axes());
    ball_histories.resize(dev.get_num_balls());


    guid = dev.get_guid();
    auto [vendor_, product_, version_, crc16_] = sdl::joystick::parse(guid);
    crc = crc16_;

    auto gc_mapping = sdl::game_controller::try_get_mapping(guid);
    if (gc_mapping) {
        cout << "found mapping: " << *gc_mapping << endl;
        mapping = parse_mapping(*gc_mapping);
        // dump_mapping(mapping);
    }
}


JoystickWindow::~JoystickWindow()
    noexcept = default;


void
JoystickWindow::process()
{
    update_history();

    ImGui::SetNextWindowSize({600, 500}, ImGuiCond_FirstUseEver);
    std::string title = "Joystick: "s
        + dev.try_get_name().value_or("")
        + "##"s
        + std::to_string(id);
    if (ImGui::Begin(title.data(), &is_open)) {

        ImGui::BeginTabBar("main_items");

        if (ImGui::BeginTabItem("Details")) {
            show_details();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Axes")) {
            show_axes();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Balls")) {
            show_balls();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Hats")) {
            show_hats();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Buttons")) {
            show_buttons();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Mapping")) {
            show_mapping();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Extras")) {
        show_extras();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    ImGui::End();

    if (!is_open)
        parent->close_later(id);
}


void
JoystickWindow::show_details()
{
    if (ImGui::BeginTable("Details", 2)) {

        ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(key_color, "Name");
        ImGui::TableNextColumn();
        auto name = dev.try_get_name();
        if (name)
            ImGui::Text("%s", *name);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(key_color, "ID");
        ImGui::TableNextColumn();
        ImGui::Text("%d", dev.get_id());

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(key_color, "Path");
        ImGui::TableNextColumn();
        auto path = dev.try_get_path();
        if (path)
            ImGui::Text("%s", *path);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(key_color, "VID:PID");
        ImGui::TableNextColumn();
        ImGui::Text("%04x:%04x (%04x)",
                    dev.get_vendor(),
                    dev.get_product(),
                    dev.get_version());

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(key_color, "Firmware");
        ImGui::TableNextColumn();
        ImGui::Text("%04x", dev.get_firmware());

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(key_color, "Serial");
        ImGui::TableNextColumn();
        auto serial = dev.try_get_serial();
        if (serial)
            ImGui::Text("%s", *serial);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(key_color, "Type");
        ImGui::TableNextColumn();
        auto type = dev.get_type();
        ImGui::Text("%s", to_string(type).data());

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(key_color, "GUID");
        ImGui::TableNextColumn();
        ImGui::Text("%s", to_string(guid).data());

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(key_color, "Player");
        ImGui::TableNextColumn();
        {
            int p = dev.get_player();
            if (ImGui::InputInt("##Player", &p))
                dev.set_player(p);
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(key_color, "Battery");
        ImGui::TableNextColumn();
        auto power = dev.get_power_level();
        ImGui::Text("%s", to_string(power).data());

        ImGui::EndTable();
    }
}


void
JoystickWindow::show_axes()
{
    if (axis_histories.empty())
        return;

    if (ImPlot::BeginPlot("Axes", {-1, -1}, ImPlotFlags_NoInputs)) {

        ImPlot::SetupAxes("Time", "Value",
                          ImPlotAxisFlags_RangeFit
                          | ImPlotAxisFlags_NoLabel
                          | ImPlotAxisFlags_NoTickLabels,
                          ImPlotAxisFlags_RangeFit);
        ImPlot::SetupAxesLimits(0, max_history_size - 1,
                                -1.1, 1.1,
                                ImPlotCond_Always);
        ImPlot::SetupFinish();

        for (unsigned i = 0; i < dev.get_num_axes(); ++i) {
            auto& history = axis_histories[i];
            if (!history.empty()) {
                std::string label = "Axis " + std::to_string(i);
                ImPlot::PlotLine(label.data(),
                                 &history[0],
                                 history.size());
            }
        }

        ImPlot::EndPlot();
    }
}


void
JoystickWindow::show_balls()
{
    if (ball_histories.empty())
        return;

    if (ImPlot::BeginPlot("Balls", {-1, -1}, ImPlotFlags_NoInputs)) {

        ImPlot::SetupAxes("X", "Y",
                          ImPlotAxisFlags_AutoFit,
                          ImPlotAxisFlags_AutoFit);
        ImPlot::SetupFinish();

        for (unsigned i = 0; i < dev.get_num_balls(); ++i) {
            auto& history = ball_histories[i];
            if (!history.empty()) {
                std::string label = "Ball " + std::to_string(i);
                ImPlot::PlotLine(label.data(),
                                 &history[0].x,
                                 &history[0].y,
                                 history.size(),
                                 0,
                                 0,
                                 sizeof history[0]);
            }
        }

        ImPlot::EndPlot();
    }
}


// HACK: there's no way to detect a plot item is hidden, so we peek inside implot_items.cpp
namespace ImPlot {
    bool
    IsItemHidden(const char* label_id);
}


void
JoystickWindow::show_hats()
{
    if (dev.get_num_hats() == 0)
        return;

    if (ImPlot::BeginPlot("Hats", {-1, -1}, ImPlotFlags_NoInputs)) {

        auto flags =
            ImPlotAxisFlags_RangeFit |
            ImPlotAxisFlags_NoLabel |
            ImPlotAxisFlags_NoTickLabels |
            ImPlotAxisFlags_NoTickMarks |
            ImPlotAxisFlags_NoGridLines;
        ImPlot::SetupAxes("X", "Y", flags, flags);
        ImPlot::SetupAxesLimits(-1.5, 1.5,
                                -1.5, 1.5,
                                ImPlotCond_Always);
        ImPlot::SetupFinish();

        for (unsigned i = 0; i < dev.get_num_hats(); ++i) {
            std::string label = "Hat " + std::to_string(i);
            auto h = dev.get_hat(i);
            auto pos = to_pos(h);

            bool has_direction = h != sdl::joystick::hat_dir::centered;
            ImPlot::SetNextMarkerStyle(has_direction
                                       ? ImPlotMarker_Diamond
                                       : ImPlotMarker_Circle,
                                       has_direction
                                       ? ImGui::GetFontSize() / 1.5
                                       : ImGui::GetFontSize() / 3.0);
            ImPlot::PlotScatter(label.data(),
                                &pos.x,
                                &pos.y,
                                1,
                                0, // ImPlotScatterFlags
                                0, // offset
                                sizeof pos);

            if (!ImPlot::IsItemHidden(label.data()) && has_direction) {
                ImPlot::PlotText(to_string(h).data(),
                                 pos.x,
                                 pos.y);
            }
        }

        ImPlot::EndPlot();
    }
}


void
JoystickWindow::show_buttons()
{
    if (dev.get_num_buttons() == 0)
        return;

    ImGui::BeginDisabled(true);

    const unsigned n_columns = 8;
    const unsigned n_buttons = dev.get_num_buttons();

    if (ImGui::BeginTable("Buttons", n_columns)) {
        for (unsigned i = 0; i < n_buttons; ++i) {
            std::string label = std::to_string(i);
            bool value = dev.get_button(i);
            ImGui::TableNextColumn();
            ImGui::RadioButton(label.data(), value);
        }

        ImGui::EndTable();
    }

    ImGui::EndDisabled();
}


void
JoystickWindow::show_extras()
{
    ImGui::TextColored(key_color, "Rumble");
    ImGui::Indent();
    ImGui::BeginDisabled(!dev.has_rumble());
    if (ImGui::Button("Low Freq"))
        dev.rumble(0xffff, 0, 250ms);
    ImGui::SameLine();
    if (ImGui::Button("High Freq"))
        dev.rumble(0, 0xffff, 250ms);
    ImGui::EndDisabled();

    ImGui::BeginDisabled(!dev.has_rumble_on_triggers());
    if (ImGui::Button("Left Trigger"))
        dev.rumble_triggers(1.0, 0.0, 250ms);
    ImGui::SameLine();
    if (ImGui::Button("Right Trigger"))
        dev.rumble_triggers(0.0, 1.0, 250ms);
    ImGui::EndDisabled();
    ImGui::Unindent();

    ImGui::Separator();

    // Show LED
    ImGui::TextColored(key_color, "LED");
    ImGui::Indent();
    ImGui::BeginDisabled(!dev.has_led());
    {
        if (ImGui::ColorEdit3("LED",
                              led_rgb,
                              ImGuiColorEditFlags_NoAlpha))
            dev.set_led(sdl::color::from_rgb(led_rgb[0], led_rgb[1], led_rgb[3]));
    }
    ImGui::EndDisabled();
    ImGui::Unindent();
}


void
JoystickWindow::show_mapping()
{
    using std::cout;
    using std::endl;

    bool use_crc = mapping.contains("crc");

    if (ImGui::BeginTable("Mappings", 2)) {

        ImGui::TableSetupColumn("Game Controller", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Joystick", ImGuiTableColumnFlags_WidthStretch);

        // GUID
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(key_color, "GUID");
        ImGui::TableNextColumn();
        ImGui::Text("%s", to_string(guid).data());

        // Name
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(key_color, "name");
        ImGui::TableNextColumn();
        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::InputText("##name", &mapping["name"]);
        ImGui::PopItemWidth();

        // Platform
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(key_color, "Platform");
        ImGui::TableNextColumn();
        ImGui::Text("%s", SDL_GetPlatform());

        // CRC16
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(key_color, "crc");
        ImGui::TableNextColumn();
        ImGui::Text("%04x", crc);
        ImGui::SameLine();
        if (ImGui::Checkbox("use crc", &use_crc)) {
            if (use_crc) {
                char buf[8];
                std::snprintf(buf, sizeof buf, "%04x", crc);
                mapping["crc"] = buf;
            } else {
                mapping.erase("crc");
            }
        }

        // Axes
        using sdl::game_controller::axis;
        for (int ai = 0; ai < convert(axis::max); ++ai) {
            axis a = static_cast<axis>(ai);
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            auto dst = to_string(a);
            ImGui::PushID(dst.data());
            ImGui::TextColored(key_color, "%s", dst.data());

            ImGui::TableNextColumn();
            std::string src;
            if (mapping.contains(dst))
                src = mapping.at(dst);
            std::string src_label = src;
            if (!src.empty())
                src_label += " (" + get_input(src) + ")";
            show_inputs_combo(dst, src, src_label);
            ImGui::PopID();
        }

        // Buttons
        using sdl::game_controller::button;
        for (int bi = 0; bi < convert(button::max); ++bi) {
            button b = static_cast<button>(bi);
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            auto dst = to_string(b);
            ImGui::PushID(dst.data());
            ImGui::TextColored(key_color, "%s", dst.data());

            ImGui::TableNextColumn();
            std::string src;
            if (mapping.contains(dst))
                src = mapping.at(dst);
            std::string src_label = src;
            if (!src.empty())
                src_label += " (" + get_input(src) + ")";
            show_inputs_combo(dst, src, src_label);
            ImGui::PopID();
        }

        ImGui::EndTable();
    }


    if (ImGui::Button("Clear")) {
        auto name = mapping["name"];
        mapping.clear();
        mapping["name"] = name;
        mapping["guid"] = to_string(guid);
        mapping["platform"] = SDL_GetPlatform();
    }

    ImGui::SameLine();

    if (ImGui::Button("Revert")) {
        mapping.clear();
        auto gc_mapping = sdl::game_controller::try_get_mapping(guid);
        if (gc_mapping)
            mapping = parse_mapping(*gc_mapping);
        if (use_crc)
            mapping["crc"] = crc;
    }

    ImGui::SameLine();

    std::string mapping_str = build_mapping(mapping);

    ImGui::BeginDisabled(mapping.empty());
    if (ImGui::Button("Apply")) {
        if (!mapping_str.empty()) {
            auto status = sdl::game_controller::try_add_mapping(mapping_str);
            if (status) {
                if (*status)
                    cout << "Added mapping." << endl;
                else
                    cout << "Replaced mapping." << endl;
            } else {
                cout << "Failed to add mapping: "
                     << status.error().what()
                     << endl;
            }
        }
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    if (ImGui::Button("Copy"))
        sdl::clipboard::set_text(mapping_str);

    ImGui::TextWrapped("%s", mapping_str.data());
}


void
JoystickWindow::show_inputs_combo(const std::string& dst,
                                  const std::string& src,
                                  const std::string& src_label)
{
    using std::string;

    ImGui::PushItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##Joystick inputs", src_label.data())) {
        // Special: <NONE>
        if (ImGui::Selectable("<NONE>", src.empty()))
            mapping.erase(dst);

        // List all axes.
        for (unsigned i = 0; i < dev.get_num_axes(); ++i) {
            string name = "a" + std::to_string(i);
            bool selected = src == name;
            string label = name + " (" + get_input(name) + ")";
            if (ImGui::Selectable(label.data(), selected))
                mapping[dst] = name;
        }

        // List all buttons.
        for (unsigned i = 0; i < dev.get_num_buttons(); ++i) {
            string name = "b" + std::to_string(i);
            bool selected = src == name;
            string label = name + " (" + get_input(name) + ")";
            if (ImGui::Selectable(label.data(), selected))
                mapping[dst] = name;
        }

        // List all hats.
        using sdl::joystick::hat_dir;
        for (unsigned i = 0; i < dev.get_num_hats(); ++i) {
            for (hat_dir d : {
                    hat_dir::up,
                    hat_dir::right_up,
                    hat_dir::right,
                    hat_dir::down_right,
                    hat_dir::down,
                    hat_dir::down_left,
                    hat_dir::left,
                    hat_dir::left_up
                }) {
                string name = "h" + std::to_string(i)
                    + "." + std::to_string(static_cast<unsigned>(d));
                bool selected = src == name;
                string label = name + " (" + get_input(name) + ")";
                if (ImGui::Selectable(label.data(), selected))
                    mapping[dst] = name;
            }
        }

        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();
}


std::string
JoystickWindow::get_input(const std::string& name)
{
    if (name.size() < 2)
        return "";
    if (name[0] == 'a') {
        unsigned long index = std::stoul(name.substr(1));
        char buf[64];
        std::snprintf(buf, sizeof buf, "%+.03f", dev.get_axis(index));
        return buf;
        //return std::to_string(dev.get_axis(index));
    } else if (name[0] == 'b') {
        unsigned long index = std::stoul(name.substr(1));
        return dev.get_button(index) ? "on" : "off";
    } else if (name[0] == 'h') {
        unsigned hat, val;
        int r = std::sscanf(name.data(), "h%u.%u", &hat, &val);
        if (r != 2)
            return "";
        auto cur_hat = dev.get_hat(hat);
        return convert(cur_hat) == val ? "on" : "off";
    } else
        return "";
}


void
JoystickWindow::update_history()
{
    for (unsigned i = 0; i < dev.get_num_axes(); ++i) {
        auto value = dev.get_axis(i);
        auto& history = axis_histories[i];
        if (history.size() >= max_history_size)
            history.erase(history.begin());
        history.push_back(value);
    }

    for (unsigned i = 0; i < dev.get_num_balls(); ++i) {
        auto value = dev.get_ball(i);
        auto& history = ball_histories[i];
        if (history.size() >= max_history_size)
            history.erase(history.begin());
        history.push_back(value);
    }
}
