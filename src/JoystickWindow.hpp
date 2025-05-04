#ifndef JOYSTICKWINDOW_HPP
#define JOYSTICKWINDOW_HPP

#include <string>
#include <vector>

#include <sdl2xx/joystick.hpp>

#include "Window.hpp"


struct JoystickListWindow;


struct JoystickWindow : Window {

    JoystickListWindow* parent;

    sdl::joystick::instance_id id;

    sdl::joystick::device dev;

    float led_rgb[3] = {1, 1, 1};

    std::string mapping;

    bool is_open = true;


    using axis_samples_t = std::vector<double>;
    std::vector<axis_samples_t> axis_histories;


    using ball_samples_t = std::vector<sdl::vec2>;
    std::vector<ball_samples_t> ball_histories;


    JoystickWindow(JoystickListWindow* parent,
                   sdl::joystick::instance_id id);

    ~JoystickWindow()
        noexcept override;


    void
    process()
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
    update_history();

};

#endif
