#include <imgui.h>

#include "JoystickListWindow.hpp"

#include "JoystickWindow.hpp"


JoystickListWindow::~JoystickListWindow()
    noexcept = default;


void
JoystickListWindow::process()
{
    if (ImGui::Begin("Joysticks",
                     nullptr,
                     ImGuiWindowFlags_HorizontalScrollbar)) {

        ImGui::BeginTable("joystick_table", 5, ImGuiTableFlags_Resizable);
        ImGui::TableSetupColumn("-");
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("VID:PID");
        ImGui::TableSetupColumn("Instance");
        ImGui::TableHeadersRow();

        for (auto& [key, val] : joysticks) {
            ImGui::TableNextRow();

            ImGui::PushID(to_string(val.guid).data());

            ImGui::TableNextColumn();
            if (ImGui::Button("Open"))
                open(key);

            ImGui::TableNextColumn();
            ImGui::Text("%s", val.name.data());

            ImGui::TableNextColumn();
            ImGui::Text("%s", val.joy_path.data());

            ImGui::TableNextColumn();
            ImGui::Text("%04x:%04x", val.vendor, val.product);

            ImGui::TableNextColumn();
            ImGui::Text("%d", val.id);

            ImGui::PopID();
        }

        ImGui::EndTable();

    }
    ImGui::End();


    for (auto& [key, val] : joysticks)
        if (val.window)
            val.window->process();

    process_close_later();
}


void
JoystickListWindow::handle(const sdl::events::event& e)
{
    switch (e.type) {
        case SDL_JOYDEVICEADDED:
            add(e.jdevice.which);
            break;

        case SDL_JOYDEVICEREMOVED:
            remove(e.jdevice.which);
            break;
    }


    for (auto& [key, val] : joysticks)
        if (val.window)
            val.window->handle(e);
}


void
JoystickListWindow::add(unsigned index)
{
    namespace js = sdl::joysticks;

    Info info;

    info.name     = js::get_name(index);
    info.joy_path = js::get_path(index);
    info.vendor   = js::get_vendor(index);
    info.product  = js::get_product(index);
    info.id       = js::get_instance(index);
    info.guid     = js::get_guid(index);

    joysticks[info.id] = std::move(info);
}


void
JoystickListWindow::remove(sdl::joysticks::instance_id id)
{
    joysticks.erase(id);
}


void
JoystickListWindow::open(sdl::joysticks::instance_id id)
{
    namespace js = sdl::joysticks;

    auto& info = joysticks.at(id);
    unsigned n = js::get_num_joysticks();
    unsigned index = n;
    for (unsigned i = 0; i < n; ++i) {
        if (js::get_instance(i) == info.id) {
            index = i;
            break;
        }
    }
    if (index != n)
        info.window = std::make_unique<JoystickWindow>(this, index);
    else
        info.window.reset();
}


void
JoystickListWindow::close(sdl::joysticks::instance_id id)
{
    joysticks.at(id).window.reset();
}


void
JoystickListWindow::close_later(sdl::joysticks::instance_id id)
{
    pending_close.insert(id);
}


void
JoystickListWindow::process_close_later()
{
    for (auto id : pending_close)
        close(id);
    pending_close.clear();
}
