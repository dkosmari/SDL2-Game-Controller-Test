/*
 * SDL2 Game Controller Test - a tool to visualize SDL2 game input devices.
 * Copyright (C) 2026  Daniel K. O. <dkosmari>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef JOYSTICK_WINDOW_HPP
#define JOYSTICK_WINDOW_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <sdl2xx/joystick.hpp>

#include "Window.hpp"


struct JoystickListWindow;


struct JoystickWindow : Window {

    JoystickListWindow* parent;

    sdl::joystick::instance_id id;
    sdl::joystick::guid guid;

    sdl::joystick::device dev;

    float led_rgb[3] = {1, 1, 1};

    std::map<std::string, std::string> mapping;
    Uint16 crc;

    bool is_open = true;


    using AxisSampleVec = std::vector<double>;
    std::vector<AxisSampleVec> axis_histories;

    using BallSampleVec = std::vector<sdl::vec2>;
    std::vector<BallSampleVec> ball_histories;

    using ButtonSampleVec = std::vector<std::uint8_t>;
    std::vector<ButtonSampleVec> button_histories;


    JoystickWindow(JoystickListWindow* parent,
                   sdl::joystick::instance_id id);

    ~JoystickWindow()
        noexcept override;


    void
    process_ui()
        override;

    void
    show_details();

    void
    show_axes();

    void
    show_balls();

    void
    show_hats();

    void
    show_buttons();

    void
    show_extras();

    void
    show_mapping();


    void
    show_inputs_combo(const std::string& dst,
                      const std::string& src,
                      const std::string& src_label);


    std::string
    get_input(const std::string& name);


    void
    update_history();

};

#endif
