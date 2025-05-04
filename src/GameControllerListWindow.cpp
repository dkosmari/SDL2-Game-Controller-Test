#include <iostream>

#include <imgui.h>

#include "GameControllerListWindow.hpp"

#include "GameControllerWindow.hpp"


using std::cout;
using std::endl;


GameControllerListWindow::~GameControllerListWindow()
    noexcept = default;


void
GameControllerListWindow::process()
{
    namespace js = sdl::joystick;
    namespace gc = sdl::game_controller;

    ImGui::SetNextWindowSize({800, 200}, ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Game Controllers")) {

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
            child->process();

    process_close_later();
}


void
GameControllerListWindow::handle(const sdl::events::event& e)
{
    switch (e.type) {

        case sdl::events::type::e_controller_device_added:
            add(e.cdevice.which);
            break;

        case sdl::events::type::e_controller_device_removed:
            remove(e.cdevice.which);
            break;

    }

    for (auto& [id, child] : children)
        if (child)
            child->handle(e);
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
