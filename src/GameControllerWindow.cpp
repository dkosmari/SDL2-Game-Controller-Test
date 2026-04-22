/*
 * SDL2 Game Controller Test - a tool to visualize SDL2 game input devices.
 * Copyright (C) 2026  Daniel K. O. <dkosmari>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <array>
#include <chrono>
#include <cmath>
#include <numbers>
#include <string>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <implot.h>

#include <sdl2xx/vec2.hpp>

#include "GameControllerWindow.hpp"

#include "GameControllerListWindow.hpp"
#include "UI.hpp"


using namespace std::literals;


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
    if (ImGui::Begin(title, &is_open)) {

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
        UI::key_label("Name", true);
        ImGui::TableNextColumn();
        auto name = dev.try_get_name();
        if (name)
            ImGui::Text("%s", *name);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        UI::key_label("ID", true);
        ImGui::TableNextColumn();
        ImGui::Text("%d", dev.get_id());

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        UI::key_label("Path", true);
        ImGui::TableNextColumn();
        auto path = dev.try_get_path();
        if (path)
            ImGui::Text("%s", *path);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        UI::key_label("VID:PID", true);
        ImGui::TableNextColumn();
        ImGui::Text("%04x:%04x (%04x)",
                    dev.get_vendor(),
                    dev.get_product(),
                    dev.get_version());

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        UI::key_label("Firmware", true);
        ImGui::TableNextColumn();
        ImGui::Text("%04x", dev.get_firmware());

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        UI::key_label("Serial", true);
        ImGui::TableNextColumn();
        auto serial = dev.try_get_serial();
        if (serial)
            ImGui::Text("%s", *serial);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        UI::key_label("Type", true);
        ImGui::TableNextColumn();
        auto type = dev.get_type();
        ImGui::Text(to_string(type));

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        UI::key_label("Player", true);
        ImGui::TableNextColumn();
        {
            int p = dev.get_player();
            if (ImGui::InputInt("##Player", &p))
                dev.set_player(p);
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        UI::key_label("Mapping", true);
        ImGui::TableNextColumn();
        if (auto mapping = dev.try_get_mapping())
            ImGui::TextWrapped("%s", mapping->data());

        ImGui::EndTable();
    }
}


template<std::size_t N>
static
std::array<sdl::vec2f, N>
make_circle(double radius)
{
    std::array<sdl::vec2f, N> result;
    for (std::size_t i = 0; i < N; ++i) {
        float angle = 2 * i * std::numbers::pi_v<float> / N;
        result[i] = radius * sdl::vec2f{ std::cos(angle), std::sin(angle) };
    }
    return result;
}


void
GameControllerWindow::show_sticks()
{
    using sdl::game_controller::axis;

    if (ImPlot::BeginPlot("##Sticks", {-1, -1},
                          ImPlotFlags_NoInputs | ImPlotFlags_Equal)) {

        double axis_limit = 1.1;
        ImPlot::SetupAxisLimits(ImAxis_X1, -axis_limit, +axis_limit);
        // ImPlot::SetupAxisLimits(ImAxis_Y1, -axis_limit, +axis_limit, ImPlotCond_Always);
        ImPlotAxisFlags axis_flags = 0;
        axis_flags |= ImPlotAxisFlags_RangeFit;
        axis_flags |= ImPlotAxisFlags_NoLabel;
        ImPlot::SetupAxes("X", "Y", axis_flags, axis_flags);
        ImPlot::SetupLegend(ImPlotLocation_East, ImPlotLegendFlags_Outside);
        ImPlot::SetupFinish();

        // draw deadzone
        {
            static const auto dz_points = make_circle<32>(sdl::game_controller::axis_dead_zone);
            ImPlotSpec spec;
            spec.Stride = sizeof dz_points[0];
            spec.LineWeight = 2.0f;
            spec.FillAlpha = 0.20f;
            ImPlot::PlotPolygon("Deadzone",
                                &dz_points[0].x,
                                &dz_points[0].y,
                                dz_points.size(),
                                spec);
        }

        // draw limit
        {
            static const auto range_points = make_circle<32>(1);
            ImPlotSpec spec;
            spec.Stride = sizeof range_points[0];
            spec.LineWeight = 2.0f;
            spec.FillAlpha = 0.0f;
            ImPlot::PlotPolygon("Range",
                                &range_points[0].x,
                                &range_points[0].y,
                                range_points.size(),
                                spec);
        }


        plot_stick("Left",  axis::left_x, axis::left_y);
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
        ImPlot::SetupAxes("Trigger", "Value",
                          ImPlotAxisFlags_RangeFit
                          | ImPlotAxisFlags_NoLabel
                          | ImPlotAxisFlags_NoTickLabels
                          | ImPlotAxisFlags_NoGridLines
                          | ImPlotAxisFlags_NoTickMarks,
                          ImPlotAxisFlags_RangeFit);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -0.1, 1.1);
        ImPlot::SetupLegend(ImPlotLocation_East, ImPlotLegendFlags_Outside);
        ImPlot::SetupFinish();

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

    auto& style = ImGui::GetStyle();
    float item_width =
        ImGui::CalcTextSize("rightshoulderA").x
        + style.ItemSpacing.x
        + ImGui::GetFrameHeight();

    if (ImGui::BeginChild("Buttons")) {
        ImGui::PushStyleVar(ImGuiStyleVar_DisabledAlpha, 1.0f);
        ImGui::BeginDisabled(true);
        for (unsigned i = 0; i < convert(button::max); ++i) {
            auto b = static_cast<button>(i);
            if (!dev.has_button(b))
                continue;

            UI::flow_radio_button(to_string(b), dev.get_button(b), item_width);
        }
        ImGui::EndDisabled();
        ImGui::PopStyleVar(1);
    }
    ImGui::EndChild();
}
