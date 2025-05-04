#include <chrono>
#include <iostream>
#include <string>

#include <SDL_timer.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <implot.h>

#include <sdl2xx/game_controller.hpp>

#include "JoystickWindow.hpp"

#include "JoystickListWindow.hpp"


using namespace std::literals;


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


    // Add line breaks after each ','
    std::string
    break_mapping(const std::string& input)
    {
        using std::string;

        string output;

        string::size_type start, finish;

        // unsigned loops = 0;
        // unsigned max_loops = 100;
        start = 0;
        while ((finish = input.find(',', start)) != string::npos) {
            ++finish; // include the comma to this line
            output.append(input, start, finish - start);
            output.append(1, '\n');
            start = finish;
            // ++loops;
            // if (loops > max_loops) {
            //     std::cerr << "fuck!" << std::endl;
            //     break;
            // }
            // std::cout << "[" << output << "]" << std::endl;
        }
        output.append(input, start);
        return output;
    }


    // Remove all line breaks.
    std::string
    glue_mapping(const std::string& input)
    {
        using std::string;

        string output;

        string::size_type start, finish;

        start = 0;
        while ((finish = input.find('\n', start)) != string::npos) {
            output.append(input, start, finish - start);
            start = finish + 1; // skip over the line break
        }
        output.append(input, start);
        return output;
    }
}


JoystickWindow::JoystickWindow(JoystickListWindow* parent,
                               sdl::joystick::instance_id id) :
    parent{parent},
    id{id},
    dev{sdl::joystick::device::from_id(id)}
{
    axis_histories.resize(dev.get_num_axes());
    ball_histories.resize(dev.get_num_balls());
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
        ImGui::SetNextItemWidth(-FLT_MIN);
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
        auto guid = dev.get_guid();
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
    for (unsigned i = 0; i < dev.get_num_buttons(); ++i) {
        if (i > 0 && (i % 10 != 0))
            ImGui::SameLine();
        std::string label = "##" + std::to_string(i);
        bool value = dev.get_button(i);
        ImGui::RadioButton(label.data(), value);
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

    if (ImGui::Button("Revert")) {
        mapping.clear();
    }
    ImGui::SameLine();
    if (ImGui::Button("Apply")) {
        auto str = glue_mapping(mapping);
        auto status = sdl::game_controller::try_add_mapping(str);
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

    if (mapping.empty()) {
        auto guid = dev.get_guid();
        mapping = break_mapping(sdl::game_controller::try_get_mapping(guid).value_or(""));
    }

    ImGui::InputTextMultiline("##mapping_editor", &mapping, {-1, 0});

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
