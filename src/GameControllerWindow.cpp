#include <array>
#include <chrono>
#include <cmath>
#include <string>

#include <imgui.h>
#include <implot.h>

#include <sdl2xx/vec2.hpp>

#include "GameControllerWindow.hpp"

#include "GameControllerListWindow.hpp"


using namespace std::literals;


namespace {

    const ImVec4 key_color = {1.0, 1.0, 0.5, 1.0};

}


GameControllerWindow::GameControllerWindow(GameControllerListWindow* parent,
                                           sdl::game_controller::instance_id id) :
    Window{"Game Controller "s + std::to_string(id)},
    parent{parent},
    id{id},
    dev{sdl::game_controller::device::from_id(id)}
{
    title = "Game Controller: "s
        + dev.try_get_name().value_or("")
        + "##"s
        + std::to_string(id);
}


GameControllerWindow::~GameControllerWindow()
    noexcept = default;


void
GameControllerWindow::process_event(const sdl::events::event& e)
{
    switch (sdl::events::type{e.type}) {
        using enum sdl::events::type;

        case controller_device_remapped:
            if (e.cdevice.which == id)
                remap();
            break;

        default:
            ;
    }
}


void
GameControllerWindow::remap()
{
}


void
GameControllerWindow::process_ui()
{
    ImGui::SetNextWindowSize({1000, 650}, ImGuiCond_Appearing);
    if (ImGui::Begin(title.data(), &is_open)) {

        ImGui::BeginTabBar("main_items");

        if (ImGui::BeginTabItem("Details")) {
            if (ImGui::BeginChild("details_child"))
                show_details();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Sticks")) {
            if (ImGui::BeginChild("sticks_child"))
                show_sticks();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Triggers")) {
            if (ImGui::BeginChild("triggers_child"))
                show_triggers();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Buttons")) {
            if (ImGui::BeginChild("buttons_child"))
                show_buttons();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();

    }

    ImGui::End();

    if (!is_open)
        parent->close_later(id);
}


void
GameControllerWindow::show_details()
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
        ImGui::TextColored(key_color, "Player");
        ImGui::TableNextColumn();
        {
            int p = dev.get_player();
            if (ImGui::InputInt("##Player", &p))
                dev.set_player(p);
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextColored(key_color, "Mapping");
        ImGui::TableNextColumn();
        if (auto mapping = dev.try_get_mapping())
            ImGui::TextWrapped("%s", mapping->data());

        ImGui::EndTable();
    }
}


void
GameControllerWindow::show_sticks()
{
    using sdl::game_controller::axis;

    auto available = ImGui::GetContentRegionAvail();
    float size = std::min(available.x, available.y);
    ImGui::SetCursorPosX((available.x - size) / 2);
    if (ImPlot::BeginPlot("Sticks", {size, size}, ImPlotFlags_NoInputs)) {

        ImPlotAxisFlags axis_flags = 0;
        axis_flags |= ImPlotAxisFlags_RangeFit;
        axis_flags |= ImPlotAxisFlags_NoLabel;
        axis_flags |= ImPlotAxisFlags_NoTickLabels;

        ImPlot::SetupAxes("X", "Y", axis_flags, axis_flags);

        double axis_limit = 1.1;
        ImPlot::SetupAxesLimits(-axis_limit, axis_limit,
                                -axis_limit, axis_limit,
                                ImPlotCond_Always);

        // draw deadzone
        {
            using sdl::vec2f;
            static const float dz = sdl::game_controller::axis_dead_zone;
            static const float ddz = dz * std::sqrt(2.0f) / 2.0f;
            static const std::array<vec2f, 9> points{
                vec2f{dz, 0},
                vec2f{ddz, ddz},
                vec2f{0, dz},
                vec2f{-ddz, ddz},
                vec2f{-dz, 0},
                vec2f{-ddz, -ddz},
                vec2f{0, -dz},
                vec2f{ddz, -ddz},
                vec2f{dz, 0}
            };
            ImPlotSpec spec;
            spec.Stride = sizeof(vec2f);
            spec.LineWeight = 4.0f;
            ImPlot::PlotLine("Deadzone",
                             &points[0].x,
                             &points[0].y,
                             points.size(),
                             spec);
        }


        plot_stick("Left", axis::left_x, axis::left_y);
        plot_stick("Right", axis::right_x, axis::right_y);

        ImPlot::EndPlot();
    }
}


void
GameControllerWindow::plot_stick(const std::string& label,
                                 sdl::game_controller::axis ax,
                                 sdl::game_controller::axis ay)
{
    if (dev.has_axis(ax) && dev.has_axis(ay)) {
        double x =  dev.get_axis(ax);
        double y = -dev.get_axis(ay);
        ImPlotSpec spec;
        spec.Stride = sizeof(double);
        spec.MarkerSize = 8;
        ImPlot::PlotScatter(label.data(), &x, &y, 1, spec);
    }
}


void
GameControllerWindow::show_triggers()
{
    using sdl::game_controller::axis;

    if (ImPlot::BeginPlot("Triggers", {-1, 0}, ImPlotFlags_NoInputs)) {

        ImPlotAxisFlags axis_flags = 0;
        axis_flags |= ImPlotAxisFlags_RangeFit;
        axis_flags |= ImPlotAxisFlags_NoLabel;
        axis_flags |= ImPlotAxisFlags_NoTickLabels;

        ImPlot::SetupAxes("Trigger", "Value", axis_flags, axis_flags);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -0.1, 1.1);

        double values[2] = {0, 0};
        if (dev.has_axis(axis::trigger_left))
            values[0] = dev.get_axis(axis::trigger_left);
        if (dev.has_axis(axis::trigger_right))
            values[1] = dev.get_axis(axis::trigger_right);

        const char* labels[2] = {
            "L",
            "R"
        };
        ImPlot::PlotBarGroups(labels, values, 2, 1);

        ImPlot::EndPlot();
    }


    ImGui::BeginDisabled(!dev.has_rumble_on_triggers());
    if (ImGui::BeginTable("Rumble", 2)) {
        ImGui::TableSetupColumn("Left", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Right", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        if (ImGui::Button("Rumble##left", {-1, 0}))
            dev.rumble_triggers(1.0, 0.0, 250ms);

        ImGui::TableNextColumn();
        if (ImGui::Button("Rumble##right", {-1, 0}))
            dev.rumble_triggers(0.0, 1.0, 250ms);

        ImGui::EndTable();
    }
    ImGui::EndDisabled();

}


void
GameControllerWindow::show_buttons()
{
    using sdl::game_controller::button;

    if (ImGui::BeginTable("Buttons", 2)) {

        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthStretch);

        for (unsigned i = 0; i < convert(button::max); ++i) {
            auto b = static_cast<button>(i);
            if (!dev.has_button(b))
                continue;

            ImGui::TableNextRow();
            ImGui::PushID(i);

            ImGui::TableNextColumn();
            auto label = to_string(b);
            ImGui::TextColored(key_color, "%s", label.data());

            ImGui::TableNextColumn();
            ImGui::BeginDisabled(true);
            ImGui::RadioButton("", dev.get_button(b));
            ImGui::EndDisabled();

            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}
