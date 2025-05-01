#include <imgui.h>

#include "JoystickListWindow.hpp"

#include "JoystickWindow.hpp"


JoystickListWindow::~JoystickListWindow()
    noexcept = default;


void
JoystickListWindow::process()
{
    ImGui::SetNextWindowSize({800, 200}, ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Joysticks")) {

        ImGui::BeginTable("joystick_list", 5);
        ImGui::TableSetupColumn("##open_button", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("VID:PID", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Inst.", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        for (auto& [key, val] : joysticks) {
            ImGui::TableNextRow();

            ImGui::PushID(val.id);

            ImGui::TableNextColumn();
            if (ImGui::Button("Open"))
                open(key);

            ImGui::TableNextColumn();
            ImGui::TextUnformatted(val.name.data());

            ImGui::TableNextColumn();
            ImGui::TextUnformatted(val.path.data());

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

    info.name    = js::try_get_name(index).value_or("<NONE>");
    info.path    = js::try_get_path(index).value_or("<NONE>");
    info.vendor  = js::get_vendor(index);
    info.product = js::get_product(index);
    info.id      = js::get_id(index);
    info.guid    = js::get_guid(index);

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
        if (js::get_id(i) == info.id) {
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
