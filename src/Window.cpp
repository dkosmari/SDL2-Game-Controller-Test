/*
 * SDL2 Game Controller Test - a tool to visualize SDL2 game input devices.
 * Copyright (C) 2026  Daniel K. O. <dkosmari>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Window.hpp"


Window::Window(const std::string& title) :
    title{title}
{}


void
Window::process_event(const sdl::events::event&)
{}
