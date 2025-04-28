#ifndef JOYSTICKLISTWINDOW_HPP
#define JOYSTICKLISTWINDOW_HPP

#include <map>
#include <memory>
#include <set>
#include <string>

#include <sdl2xx/joysticks.hpp>

#include "Window.hpp"


struct JoystickListWindow : Window {

    struct Info {
        std::string name;
        std::string joy_path;
        Uint16 vendor;
        Uint16 product;
        sdl::joysticks::instance_id id;
        sdl::guid guid;

        std::unique_ptr<Window> window;
    };


    std::map<sdl::joysticks::instance_id, Info> joysticks;

    std::set<sdl::joysticks::instance_id> pending_close;


    ~JoystickListWindow()
        noexcept override;

    void
    process()
        override;

    void
    handle(const sdl::events::event& e)
        override;

    void
    add(unsigned index);

    void
    remove(sdl::joysticks::instance_id id);


    void
    open(sdl::joysticks::instance_id id);


    void
    close(sdl::joysticks::instance_id id);


    void
    close_later(sdl::joysticks::instance_id id);


    void
    process_close_later();

};

#endif
