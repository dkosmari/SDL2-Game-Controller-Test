/*
 * SDL2 Game Controller Test - a tool to visualize SDL2 game input devices.
 * Copyright (C) 2026  Daniel K. O. <dkosmari>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef GAME_CONTROLLER_WINDOW_HPP
#define GAME_CONTROLLER_WINDOW_HPP

#include <string>
#include <vector>

#include <sdl2xx/game_controller.hpp>

#include "Window.hpp"


struct GameControllerListWindow;


struct GameControllerWindow : Window {

    GameControllerListWindow* parent;

    sdl::game_controller::instance_id id;

    sdl::game_controller::device dev;

    bool is_open = true;


    GameControllerWindow(GameControllerListWindow* parent,
                         sdl::game_controller::instance_id id);

    ~GameControllerWindow()
        noexcept override;

    void
    process_event(const sdl::events::event& e)
        override;


    void
    remap();


    void
    process_ui()
        override;


    void
    show_details();

    void
    show_sticks();

    void
    plot_stick(const std::string& label,
               sdl::game_controller::axis ax,
               sdl::game_controller::axis ay);

    void
    show_triggers();

    void
    show_buttons();

};


#endif
