#ifndef JOYSTICKLISTWINDOW_HPP
#define JOYSTICKLISTWINDOW_HPP

#include <map>
#include <memory>
#include <set>
#include <string>

#include <sdl2xx/joystick.hpp>

#include "Window.hpp"


struct JoystickListWindow : Window {

    using instance_id = sdl::joystick::instance_id;

    std::map<instance_id, std::unique_ptr<Window>> children;

    std::set<instance_id> pending_close;


    JoystickListWindow();

    ~JoystickListWindow()
        noexcept;

    void
    process_ui()
        override;

    void
    process_event(const sdl::events::event& e)
        override;

    void
    add(unsigned index);

    void
    remove(instance_id id);


    void
    open(instance_id id);


    void
    close_later(instance_id id);


    void
    process_close_later();

};

#endif
