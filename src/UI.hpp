/*
 * SDL2 Game Controller Test - a tool to visualize SDL2 game input devices.
 * Copyright (C) 2026  Daniel K. O. <dkosmari>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef UI_HPP
#define UI_HPP

#include <string>

#include <imgui.h>


namespace UI {

    void
    key_label(const std::string& text,
              bool align_to_frame_padding = false);


    void
    flow_radio_button(const std::string& label,
                      bool value,
                      float width);

} // namespace UI

#endif
