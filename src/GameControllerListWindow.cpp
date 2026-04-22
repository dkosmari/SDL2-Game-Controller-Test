/*
 * SDL2 Game Controller Test - a tool to visualize SDL2 game input devices.
 * Copyright (C) 2026  Daniel K. O. <dkosmari>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <iostream>

#include <imgui.h>
#include <imgui_stdlib.h>

#include "GameControllerListWindow.hpp"

#include "GameControllerWindow.hpp"


using std::cout;
using std::endl;


GameControllerListWindow::GameControllerListWindow() :
    Window{"Game Controllers"}
{}


GameControllerListWindow::~GameControllerListWindow()
    noexcept = default;


void
GameControllerListWindow::process_ui()
{
    namespace js = sdl::joystick;
    namespace gc = sdl::game_controller;

    auto vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos({10, 10 + vp->Size.y/2}, ImGuiCond_Appearing);
    ImGui::SetNextWindowSize({vp->Size.x - 20, vp->Size.y/2 - 20}, ImGuiCond_Appearing);
    if (ImGui::Begin(title)) {

        ImGui::BeginTable("game_controller_list", 5);

        ImGui::TableSetupColumn("##open_button", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("VID:PID", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        const unsigned n = js::get_num_devices();
        for (unsigned idx = 0; idx < n; ++idx) {

            if (!gc::is_game_controller(idx))
                continue;

            auto id = gc::get_id(idx);

            ImGui::TableNextRow();

            ImGui::PushID(id);

            ImGui::TableNextColumn();
            if (ImGui::Button("Open"))
                open(id);

            ImGui::TableNextColumn();
            ImGui::Text("%d", id);

            ImGui::TableNextColumn();
            auto name = gc::try_get_name(idx);
            if (name)
                ImGui::Text("%s", *name);

            ImGui::TableNextColumn();
            auto path = gc::try_get_path(idx);
            if (path)
                ImGui::Text("%s", *path);

            ImGui::TableNextColumn();
            ImGui::Text("%04x:%04x",
                        gc::get_vendor(idx),
                        gc::get_product(idx));

            ImGui::PopID();
        }

        ImGui::EndTable();
    }
    ImGui::End();

    for (auto& [id, child] : children)
        if (child)
            child->process_ui();

    process_close_later();
}


void
GameControllerListWindow::process_event(const sdl::events::event& e)
{
    switch (sdl::events::type{e.type}) {
        using enum sdl::events::type;

        case controller_device_added:
            add(e.cdevice.which);
            break;

        case controller_device_removed:
            remove(e.cdevice.which);
            break;

        default:
            ;
    }

    for (auto& [id, child] : children)
        if (child)
            child->process_event(e);
}


void
GameControllerListWindow::add(unsigned)
{}


void
GameControllerListWindow::remove(instance_id id)
{
    children.erase(id);
}


void
GameControllerListWindow::open(instance_id id)
{
    auto& child = children[id];
    if (!child)
        child = std::make_unique<GameControllerWindow>(this, id);
}


void
GameControllerListWindow::close_later(instance_id id)
{
    pending_close.insert(id);
}


void
GameControllerListWindow::process_close_later()
{
    for (auto id : pending_close)
        remove(id);
    pending_close.clear();
}
