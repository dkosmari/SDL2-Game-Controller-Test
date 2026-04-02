/*
 * SDL2 Game Controller Test - a tool to visualize SDL2 game input devices.
 * Copyright (C) 2026  Daniel K. O. <dkosmari>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>

#include <implot.h>

#include <sdl2xx/sdl.hpp>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "App.hpp"

#include "GameControllerListWindow.hpp"
#include "JoystickListWindow.hpp"

using std::cout;
using std::cerr;
using std::endl;

using namespace sdl::literals;


namespace App {

    void
    add_custom_mappings(const std::filesystem::path& src);

    std::filesystem::path
    get_content_path();


    // RAII type to store resources associated with the liftetime of the app
    struct Resources {

        sdl::init sdl_init{sdl::init::flag::video,
                           sdl::init::flag::joystick,
                           sdl::init::flag::game_controller,
                           sdl::init::flag::sensor,
                           sdl::init::flag::haptic,
                           sdl::init::flag::audio};

#ifdef __WIIU__
        sdl::sub_init audio_init{sdl::init::flag::audio};
#endif


        sdl::window window{PACKAGE_NAME,
                           sdl::window::pos_undefined,
                           {1280, 720},
                           sdl::window::flag::resizable};

        sdl::renderer renderer{window,
                               -1,
                               sdl::renderer::flag::accelerated,
                               sdl::renderer::flag::present_vsync};

        std::vector<std::unique_ptr<Window>> children;

        std::vector<sdl::game_controller::device> controllers;
    };

    std::optional<Resources> res;

    bool running = false;


    void
    draw()
    {

        res->renderer.set_color(0x402040_rgb);
        res->renderer.clear();

        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(),
                                              res->renderer.data());

        res->renderer.present();
    }


    void
    process_ui()
        noexcept
    {
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // gui code goes here
        try {
            for (auto& c : res->children)
                c->process_ui();
        }
        catch (std::exception& e) {
            cout << "Error in GUI code: " << e.what() << endl;
        }

        // ImGui::ShowDemoWindow();

        ImGui::EndFrame();
        ImGui::Render();
    }


    void
    process_events()
    {
        sdl::events::event event;
        while (sdl::events::poll(event)) {

            ImGui_ImplSDL2_ProcessEvent(&event);

            for (auto& c : res->children)
                c->process_event(event);

            switch (sdl::events::type{event.type}) {
                using enum sdl::events::type;

                case quit:
                    running = false;
                    break;

                case controller_device_added:
                    res->controllers.emplace_back(event.cdevice.which);
                    break;

                case controller_device_removed:
                    std::erase_if(res->controllers,
                                  [id=event.cdevice.which](sdl::game_controller::device& gc)
                                  {
                                      return id == gc.get_id();
                                  });
                    break;

                default:
                    ;
            }

        }
    }


    void
    add_custom_mappings(const std::filesystem::path& src)
    {
        if (!exists(src))
            return;

        auto db_path = src / "gamecontrollerdb.txt";
        if (exists(db_path)) {
            if (auto status = sdl::game_controller::try_add_mappings(db_path))
                cout << "Added custom controller database: " << db_path << endl;
            else
                cout << "Error reading controller database from "
                     << db_path << ":\n"
                     << "    " << status.error().what()
                     << endl;
        }

        auto mappings_path = src / "mappings";
        if (exists(mappings_path) && is_directory(mappings_path)) {
            try {
                for (auto entry : std::filesystem::directory_iterator{mappings_path}) {
                    if (!entry.is_regular_file())
                        continue;
                    auto p = entry.path();
                    if (p.extension() != ".txt" && p.extension() != ".csv")
                        continue;

                    if (auto status = sdl::game_controller::try_add_mappings(p))
                        cout << "Added custom controller mapping: " << p << endl;
                    else
                        cout << "Error adding mapping from "
                             << p << ":\n"
                             << "    " << status.error().what()
                             << endl;
                }
            }
            catch (std::exception& e) {
                cout << "Did not add custom mappings: " << e.what() << endl;
            }
        }
    }


    std::filesystem::path
    get_content_path()
    {
#ifdef __WIIU__
        return"/vol/content";
#else
        return "assets/content";
#endif
    }

} // namespace App



// public functions

void
App::initialize()
{

    res.emplace();

#ifdef __WIIU__
    // Create a temporary audio device to stop the boot sound.
    sdl::audio::spec aspec;
    aspec.freq = 32000;
    aspec.format = AUDIO_S16SYS;
    aspec.channels = 2;
    aspec.samples = 2048;
    sdl::audio::device adev{nullptr, false, aspec};
#endif

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    auto& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    io.LogFilename = nullptr; // don't save log
    io.IniFilename = nullptr; // don't save ini

    io.ConfigDragScroll = true;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.MouseDragThreshold = 25;

    auto& style = ImGui::GetStyle();
    style.ScaleAllSizes(3);

    auto font_path = get_content_path() / "DejaVuSans.ttf";
    io.Fonts->AddFontFromFileTTF(font_path.c_str(), 30);

    ImGui_ImplSDL2_InitForSDLRenderer(res->window.data(),
                                      res->renderer.data());
    ImGui_ImplSDLRenderer2_Init(res->renderer.data());

    add_custom_mappings(get_content_path());

    res->children.push_back(std::make_unique<JoystickListWindow>());
    res->children.push_back(std::make_unique<GameControllerListWindow>());

    // unsigned n_sensors = sdl::sensor::get_num_devices();
    // cout << "num sensors: " << n_sensors << endl;
}


void
App::finalize()
    noexcept
{
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();

    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    res.reset();
}


int
App::run()
{
    try {
        running = true;
        while (running) {
            process_ui();
            draw();
            process_events();
        }
        return 0;
    }
    catch (...) {
        cerr << "Exception while running!" << endl;
        throw;
    }
}
