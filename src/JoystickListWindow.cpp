#include <imgui.h>

#include "JoystickListWindow.hpp"


JoystickListWindow::~JoystickListWindow()
    noexcept = default;


void
JoystickListWindow::process(App& /*app*/)
{
    if (ImGui::Begin("Joysticks")) {

        ImGui::BeginTable("joystick_table", 9);
        ImGui::TableSetupColumn("Index");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Player");
        ImGui::TableSetupColumn("Path");
        ImGui::TableSetupColumn("Vendor");
        ImGui::TableSetupColumn("Product");
        ImGui::TableSetupColumn("Version");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Instance");
        ImGui::TableHeadersRow();

        for (auto [key, val] : joysticks) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%u", val.index);

            ImGui::TableNextColumn();
            ImGui::Text("%s", val.name.data());

            ImGui::TableNextColumn();
            ImGui::Text("%d", val.player);

            ImGui::TableNextColumn();
            ImGui::Text("%s", val.joy_path.data());

            ImGui::TableNextColumn();
            ImGui::Text("%u", val.vendor);

            ImGui::TableNextColumn();
            ImGui::Text("%u", val.product);

            ImGui::TableNextColumn();
            ImGui::Text("%u", val.version);

            ImGui::TableNextColumn();
            ImGui::Text("%u", val.type);

            ImGui::TableNextColumn();
            ImGui::Text("%d", val.instance);
        }

        ImGui::EndTable();

    }
    ImGui::End();
}


void
JoystickListWindow::handle(sdl::events::event& e)
{
    switch (e.type) {
        case SDL_JOYDEVICEADDED:
            add(e.jdevice.which);
            break;

        case SDL_JOYDEVICEREMOVED:
            remove(e.jdevice.which);
            break;
    }
}


void
JoystickListWindow::add(Sint32 index)
{
    Info info;
    info.index = index;
    auto name = SDL_JoystickNameForIndex(index);
    info.name = name ? name : "<NULL>";
    info.player = SDL_JoystickGetDevicePlayerIndex(index);
    info.joy_path = SDL_JoystickPathForIndex(index);
    info.vendor = SDL_JoystickGetDeviceVendor(index);
    info.product = SDL_JoystickGetDeviceProduct(index);
    info.version = SDL_JoystickGetDeviceProductVersion(index);
    info.type = SDL_JoystickGetDeviceType(index);
    info.instance = SDL_JoystickGetDeviceInstanceID(index);
    joysticks[info.instance] = std::move(info);
}


void
JoystickListWindow::remove(Sint32 id)
{
    joysticks.erase(id);
}
