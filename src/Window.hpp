#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <string>

#include <sdl2xx/events.hpp>


struct Window {

    std::string title;

    Window(const std::string& title);

    virtual ~Window() noexcept = default;

    virtual
    void
    process_ui() = 0;

    virtual
    void
    process_event(const sdl::events::event& e);

};

#endif
