#ifndef APP_HPP
#define APP_HPP

#include <memory>
#include <vector>

#include <sdl2xx/sdl.hpp>

#include "Window.hpp"


class Window;


struct App {

    sdl::init sdl_init;
    sdl::window window;
    sdl::renderer renderer;
    std::vector<std::unique_ptr<Window>> windows;
    bool running = false;


    App();

    ~App();

    int
    run();

    void
    draw();

    void
    process_ui();

    void
    process_events();

};

#endif
