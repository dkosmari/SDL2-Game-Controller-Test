#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <sdl2xx/events.hpp>


struct Window {

    virtual ~Window() noexcept = default;

    virtual
    void
    process() = 0;

    virtual
    void
    handle(const sdl::events::event& e) = 0;

};

#endif
