#ifndef JOYSTICKWINDOW_HPP
#define JOYSTICKWINDOW_HPP

#include <string>
#include <utility>
#include <vector>

#include <sdl2xx/joysticks.hpp>

#include "Window.hpp"


struct JoystickListWindow;


struct JoystickWindow : Window {

    JoystickListWindow* parent = nullptr;

    sdl::joysticks::joystick joy;

    sdl::joysticks::instance_id id;
    std::string name;
    int player;
    std::string path;
    Uint16 vendor;
    Uint16 product;
    Uint16 version;
    sdl::joysticks::type type;
    sdl::guid guid;

    bool is_open = true;


    using axis_samples_t = std::vector<Sint16>;
    axis_samples_t current_axis;
    std::vector<axis_samples_t> axis_histories;


    using ball_samples_t = std::vector<sdl::vec2>;
    ball_samples_t current_ball;
    std::vector<ball_samples_t> ball_histories;


    using hat_samples_t = std::vector<sdl::joysticks::hat_dir>;
    hat_samples_t current_hat;


    using button_samples_t = std::vector<bool>;
    button_samples_t current_button;

    sdl::joysticks::power_level battery;


    JoystickWindow(JoystickListWindow* parent,
                   unsigned index);

    ~JoystickWindow()
        noexcept override;


    void
    process()
        override;


    void
    handle(const sdl::events::event& e)
        override;


    void
    handle(const sdl::events::joy_axis& e);

    void
    handle(const sdl::events::joy_ball& e);

    void
    handle(const sdl::events::joy_battery& e);

    void
    handle(const sdl::events::joy_button& e);

    void
    handle(const sdl::events::joy_hat& e);


    void
    update_history();

};

#endif
