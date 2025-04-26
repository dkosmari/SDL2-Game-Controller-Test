#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <sdl2xx/events.hpp>


class App;


struct Window {

    virtual ~Window() noexcept = default;

    virtual
    void
    process(App& app) = 0;

    virtual
    void
    handle(sdl::events::event& e) = 0;

};

#endif
