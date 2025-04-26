#ifndef JOYSTICKLISTWINDOW_HPP
#define JOYSTICKLISTWINDOW_HPP

#include <map>
#include <string>

#include <SDL.h>

#include "Window.hpp"


struct JoystickListWindow : Window {

    struct Info {
        unsigned index;
        std::string name;
        int player;
        std::string joy_path;
        Uint16 vendor;
        Uint16 product;
        Uint16 version;
        SDL_JoystickType type;
        SDL_JoystickID instance;
    };


    std::map<SDL_JoystickID, Info> joysticks;


    ~JoystickListWindow()
        noexcept override;

    void
    process(App& app)
        override;

    void
    handle(sdl::events::event& e);

    void
    add(Sint32 index);

    void
    remove(Sint32 id);
};

#endif
