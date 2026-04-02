/*
 * SDL2 Game Controller Test - a tool to visualize SDL2 game input devices.
 * Copyright (C) 2026  Daniel K. O. <dkosmari>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef APP_HPP
#define APP_HPP

namespace App {

    void
    initialize();

    void
    finalize()
        noexcept;

    int
    run();

} // namespace App

#endif
