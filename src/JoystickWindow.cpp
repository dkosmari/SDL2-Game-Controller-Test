#include <chrono>
#include <iostream>

#include <SDL_timer.h>

#include <imgui.h>
#include <implot.h>

#include "JoystickWindow.hpp"

#include "JoystickListWindow.hpp"


// #define DEBUG_BALLS

// #define DEBUG_HATS

using std::cout;
using std::endl;

using namespace std::literals;


namespace {

    const unsigned max_history = 60 * 5;

    sdl::vec2
    to_pos(sdl::joysticks::hat_dir dir)
    {
        using sdl::joysticks::hat_dir;
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

}


JoystickWindow::JoystickWindow(JoystickListWindow* parent,
                               unsigned index) :
    parent{parent},
    joy{index},
    id{joy.get_instance()},
    name{joy.try_get_name().value_or(nullptr)},
    player{joy.get_player_index()},
    path{joy.try_get_path().value_or(nullptr)},
    vendor{joy.get_vendor()},
    product{joy.get_product()},
    version{joy.get_product_version()},
    firmware{joy.get_firmware_version()},
    serial{joy.try_get_serial().value_or(nullptr)},
    type{joy.get_type()},
    guid{joy.get_guid()}
{
    {
        const unsigned num_axes = joy.get_num_axes();
        current_axis.resize(num_axes);
        for (unsigned i = 0; i < num_axes; ++i)
            current_axis[i] = joy.get_axis(i);
        axis_histories.resize(num_axes);
    }

    {
#ifdef DEBUG_BALLS
        const unsigned num_balls = joy.get_num_axes() / 2;
        current_ball.resize(num_balls);
        for (unsigned i = 0; i < num_balls; ++i)
            current_ball[i] = {0, 0};
        ball_histories.resize(num_balls);
#else
        const unsigned num_balls = joy.get_num_balls();
        current_ball.resize(num_balls);
        for (unsigned i = 0; i < num_balls; ++i)
            current_ball[i] = joy.get_ball(i);
        ball_histories.resize(num_balls);
#endif
    }

    {
#ifdef DEBUG_HATS
        const unsigned num_hats = 1;
        current_hat.resize(num_hats);
#else
        const unsigned num_hats = joy.get_num_hats();
        current_hat.resize(num_hats);
        for (unsigned i = 0; i < num_hats; ++i)
            current_hat[i] = joy.get_hat(i);
#endif
    }

    {
        const unsigned num_buttons = joy.get_num_buttons();
        current_button.resize(num_buttons);
        for (unsigned i = 0; i < num_buttons; ++i)
            current_button[i] = joy.get_button(i);
    }

    battery = joy.get_power_level();
}


JoystickWindow::~JoystickWindow()
    noexcept
{}


void
JoystickWindow::process()
{
    update_history();

    if (ImGui::Begin(("Joystick: "s + name).c_str(),
                     &is_open)) {

        if (name) {
            ImGui::Text("Name:");
            ImGui::Indent();
            ImGui::Text("%s", name);
            ImGui::Unindent();
        }
        ImGui::Text("Instance: %d", id);
        if (path)
            ImGui::Text("Path: %s", path);
        ImGui::Text("VID:PID: %04x:%04x (%04x)", vendor, product, version);
        if (firmware)
            ImGui::Text("Firmware: %04x", firmware);
        if (serial)
            ImGui::Text("Serial: %s", serial);
        ImGui::Text("Type: %s", to_string(type).data());
        ImGui::Text("GUID: %s", to_string(guid).data());

        {
            int p = joy.get_player_index();
            if (ImGui::InputInt("Player", &p))
                joy.set_player_index(p);
        }

        // Plot axes.
        if (!axis_histories.empty()) {
            if (ImPlot::BeginPlot("Axes",
                                  {-1, 0},
                                  ImPlotFlags_NoInputs)) {

                ImPlot::SetupAxes("Time", "Value",
                                  ImPlotAxisFlags_RangeFit
                                  | ImPlotAxisFlags_NoLabel
                                  | ImPlotAxisFlags_NoTickLabels,
                                  ImPlotAxisFlags_RangeFit);
                ImPlot::SetupAxisLimits(ImAxis_X1,
                                        0,
                                        max_history - 1,
                                        ImPlotCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1,
                                        sdl::joysticks::axis_min - 1024,
                                        sdl::joysticks::axis_max + 1024,
                                        ImPlotCond_Always);
                ImPlot::SetupFinish();

                for (std::size_t i = 0; i < axis_histories.size(); ++i) {
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

        // Plot balls.
        if (!ball_histories.empty()) {
            if (ImPlot::BeginPlot("Balls",
                                  {-1, 0},
                                  ImPlotFlags_NoInputs)) {

                ImPlot::SetupAxes("X", "Y",
                                  ImPlotAxisFlags_AutoFit,
                                  ImPlotAxisFlags_AutoFit);
                ImPlot::SetupFinish();

                for (std::size_t i = 0; i < ball_histories.size(); ++i) {
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

        // Plot hats.
        if (!current_hat.empty()) {
            if (ImPlot::BeginPlot("Hats",
                                  {-1, 0},
                                  ImPlotFlags_NoInputs)) {

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

                for (std::size_t i = 0; i < current_hat.size(); ++i) {
                    std::string label = "Hat " + std::to_string(i);
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond,
                                               16);
                    auto pos = to_pos(current_hat[i]);
                    ImPlot::PlotScatter(label.data(),
                                        &pos.x,
                                        &pos.y,
                                        1,
                                        0, // ImPlotScatterFlags
                                        0, // offset
                                        sizeof pos);
                }

                ImPlot::EndPlot();
            }
        }

        // Show buttons.
        if (!current_button.empty()) {
            ImGui::Text("Buttons");
            ImGui::BeginGroup();
            ImGui::Indent();
            ImGui::BeginDisabled(true);
            for (std::size_t i = 0; i < current_button.size(); ++i) {
                if (i > 0 && (i % 10 != 0))
                    ImGui::SameLine();
                std::string label = "##" + std::to_string(i);
                bool value = current_button[i];
                ImGui::RadioButton(label.data(), value);
            }
            ImGui::EndDisabled();
            ImGui::Unindent();
            ImGui::EndGroup();
        }

        // Show battery.
        ImGui::Text("Battery: %s", to_string(battery).data());


        // Show rumble.
        ImGui::BeginDisabled(!joy.has_rumble());
        if (ImGui::Button("Rumble"))
            joy.rumble(128, 0, 250ms);
        ImGui::EndDisabled();

        ImGui::SameLine();

        ImGui::BeginDisabled(!joy.has_rumble_on_triggers());
        if (ImGui::Button("Rumble Triggers"))
            joy.rumble_triggers(128, 128, 250ms);
        ImGui::EndDisabled();

        // Show LED
        ImGui::BeginDisabled(!joy.has_led());
        {
            if (ImGui::ColorEdit3("LED",
                                  led_rgb,
                                  ImGuiColorEditFlags_NoAlpha))
                joy.set_led(sdl::color::from_rgb(led_rgb[0], led_rgb[1], led_rgb[3]));
        }
        ImGui::EndDisabled();
    }
    ImGui::End();

    if (!is_open)
        parent->close_later(id);
}


void
JoystickWindow::handle(const sdl::events::event& e)
{
    switch (e.type) {

        case SDL_JOYAXISMOTION:
            handle(e.jaxis);
            break;

        case SDL_JOYBALLMOTION:
            handle(e.jball);
            break;

        case SDL_JOYBATTERYUPDATED:
            handle(e.jbattery);
            break;

        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
            handle(e.jbutton);
            break;

        case SDL_JOYHATMOTION:
            handle(e.jhat);
            break;

    }
}


void
JoystickWindow::handle(const sdl::events::joy_axis& e)
{
    if (e.which != id)
        return;

#ifdef DEBUG_BALLS
    // generate fake ball movement from the axes
    int delta = current_axis[e.axis] - e.value;
    unsigned ball = e.axis / 2;
    if ((e.axis & 1) == 0) {
        current_ball[ball].x = delta;
    } else {
        current_ball[ball].y = delta;
    }
#endif


#ifdef DEBUG_HATS
    using sdl::joysticks::hat_dir;
    Uint8 h = current_hat[0];
    if (e.axis == 0) {
        h &= ~hat_dir::left;
        h &= ~hat_dir::right;
        if (e.value < sdl::joysticks::axis_min / 2)
            h |= hat_dir::left;
        if (e.value > sdl::joysticks::axis_max / 2)
            h |= hat_dir::right;
    }
    if (e.axis == 1) {
        h &= ~hat_dir::up;
        h &= ~hat_dir::down;
        if (e.value < sdl::joysticks::axis_min / 2)
            h |= hat_dir::up;
        if (e.value > sdl::joysticks::axis_max / 2)
            h |= hat_dir::down;
    }
    current_hat[0] = static_cast<hat_dir>(h);
#endif

    current_axis[e.axis] = e.value;
}


void
JoystickWindow::handle(const sdl::events::joy_ball& e)
{
    if (e.which != id)
        return;
    current_ball[e.ball] = {e.xrel, e.yrel};
}


void
JoystickWindow::handle(const sdl::events::joy_battery& e)
{
    if (e.which != id)
        return;
    battery = static_cast<sdl::joysticks::power_level>(e.level);
}


void
JoystickWindow::handle(const sdl::events::joy_button& e)
{
    if (e.which != id)
        return;
    current_button[e.button] = e.state;
}


void
JoystickWindow::handle(const sdl::events::joy_hat& e)
{
    if (e.which != id)
        return;
    current_hat[e.hat] = static_cast<sdl::joysticks::hat_dir>(e.value);
}


void
JoystickWindow::update_history()
{
    for (unsigned i = 0; i < current_axis.size(); ++i) {
        auto value = current_axis[i];
        auto& history = axis_histories[i];
        if (history.size() >= max_history)
            history.erase(history.begin());
        history.push_back(value);
    }

    for (unsigned i = 0; i < current_ball.size(); ++i) {
        auto value = current_ball[i];
        auto& history = ball_histories[i];
        if (history.size() >= max_history)
            history.erase(history.begin());
        history.push_back(value);
    }

}
