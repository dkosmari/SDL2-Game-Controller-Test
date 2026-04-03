/*
 * SDL2 Game Controller Test - a tool to visualize SDL2 game input devices.
 * Copyright (C) 2026  Daniel K. O. <dkosmari>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <string>

#include <sdl2xx/events.hpp>


struct Window {

    std::string title;

    Window(const std::string& title);

    virtual ~Window() noexcept = default;

    virtual
    void
    process_ui() = 0;

    virtual
    void
    process_event(const sdl::events::event& e);

};

#endif
