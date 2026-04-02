#include <imgui.h>

#include "JoystickListWindow.hpp"

#include "JoystickWindow.hpp"


JoystickListWindow::JoystickListWindow() :
    Window{"Joysticks"}
{}


JoystickListWindow::~JoystickListWindow()
    noexcept = default;


void
JoystickListWindow::process_ui()
{
    namespace js = sdl::joystick;

    auto vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos({10, 10}, ImGuiCond_Appearing);
    ImGui::SetNextWindowSize({vp->Size.x - 20, vp->Size.y/2 - 20}, ImGuiCond_Appearing);
    if (ImGui::Begin(title.data())) {

        ImGui::BeginTable("joystick_list", 5);

        ImGui::TableSetupColumn("##open_button", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("VID:PID", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        for (unsigned idx = 0; idx < js::get_num_devices(); ++idx) {
            ImGui::TableNextRow();

            auto id = js::get_id(idx);

            ImGui::PushID(id);

            ImGui::TableNextColumn();
            if (ImGui::Button("Open"))
                open(id);

            ImGui::TableNextColumn();
            ImGui::Text("%d", id);

            ImGui::TableNextColumn();
            auto name = js::try_get_name(idx);
            if (name)
                ImGui::Text("%s", *name);

            ImGui::TableNextColumn();
            auto path = js::try_get_path(idx);
            if (path)
                ImGui::Text("%s", *path);

            ImGui::TableNextColumn();
            ImGui::Text("%04x:%04x",
                        js::get_vendor(idx),
                        js::get_product(idx));

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
JoystickListWindow::process_event(const sdl::events::event& e)
{
    switch (sdl::events::type{e.type}) {
        using enum sdl::events::type;

        case joy_device_added:
            add(e.jdevice.which);
            break;

        case joy_device_removed:
            remove(e.jdevice.which);
            break;

        default:
            ;

    }

    for (auto& [id, child] : children)
        if (child)
            child->process_event(e);
}


void
JoystickListWindow::add(unsigned)
{}


void
JoystickListWindow::remove(instance_id id)
{
    children.erase(id);
}


void
JoystickListWindow::open(instance_id id)
{
    auto& child = children[id];
    if (!child)
        child = std::make_unique<JoystickWindow>(this, id);
}


void
JoystickListWindow::close_later(instance_id id)
{
    pending_close.insert(id);
}


void
JoystickListWindow::process_close_later()
{
    for (auto id : pending_close)
        remove(id);
    pending_close.clear();
}
