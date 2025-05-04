#ifndef GAMECONTROLLERLISTWINDOW_HPP
#define GAMECONTROLLERLISTWINDOW_HPP

#include <map>
#include <memory>
#include <set>
#include <string>

#include <sdl2xx/game_controller.hpp>

#include "Window.hpp"


struct GameControllerListWindow : Window {

    using instance_id = sdl::joystick::instance_id;


    std::map<instance_id, std::unique_ptr<Window>> children;
    std::set<instance_id> pending_close;


    ~GameControllerListWindow()
        noexcept;


    void
    process()
        override;

    void
    handle(const sdl::events::event& e)
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
