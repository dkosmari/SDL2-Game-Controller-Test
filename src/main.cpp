/*
 * SDL2 Game Controller Test - a tool to visualize SDL2 game input devices.
 * Copyright (C) 2026  Daniel K. O. <dkosmari>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <cstdlib>
#include <exception>
#include <iostream>

#include "App.hpp"

using std::cout;
using std::endl;


int main(int, char* [])
{
    int result;
    try {
        App::initialize();
        result = App::run();
    }
    catch (std::exception& e) {
        cout << "ERROR: " << e.what() << endl;
        result = EXIT_FAILURE;
    }
    App::finalize();
    return result;
}
