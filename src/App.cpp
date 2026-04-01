#include <filesystem>
#include <iostream>

#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>

#include <implot.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "App.hpp"

#include "GameControllerListWindow.hpp"
#include "JoystickListWindow.hpp"

using std::filesystem::path;
using std::cout;
using std::cerr;
using std::endl;

using namespace sdl::literals;

#ifdef __WIIU__
path assets_path = "/vol/content";
#else
path assets_path = "assets/content";
#endif


App::App() :
    sdl_init{sdl::init::flag::video,
             sdl::init::flag::joystick,
             sdl::init::flag::game_controller,
             sdl::init::flag::sensor,
             sdl::init::flag::haptic,
             sdl::init::flag::audio},
    window{PACKAGE_NAME,
           sdl::window::pos_undefined,
           {1280, 720},
           sdl::window::flag::resizable},
    renderer{window,
             -1,
             sdl::renderer::flag::accelerated,
             sdl::renderer::flag::present_vsync}
{

    // Create a temporary audio device to stop the boot sound.
    sdl::audio::spec aspec;
    aspec.freq = 32000;
    aspec.format = AUDIO_S16SYS;
    aspec.channels = 2;
    aspec.samples = 2048;
    sdl::audio::device adev{nullptr, false, aspec};

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
    io.ConfigInputTrickleEventQueue = false;
    auto& style = ImGui::GetStyle();
    style.ScaleAllSizes(2);

    auto font_path = assets_path / "DejaVuSans.ttf";
    io.Fonts->AddFontFromFileTTF(font_path.c_str(), 30);

    sdl::game_controller::try_add_mappings(assets_path / "gamecontrollerdb.txt");
    try {
        for (auto entry : std::filesystem::directory_iterator{assets_path / "mappings"}) {
            if (!entry.is_regular_file())
                continue;
            auto p = entry.path();
            if (p.extension() != ".txt" && p.extension() != ".csv")
                continue;
            try {
                sdl::game_controller::add_mappings(p);
            }
            catch (std::exception& e) {
                cout << "Error adding mappings for " << p << ": " << e.what() << endl;
            }
        }
    }
    catch (std::exception& e) {
        cout << "Did not add custom mappings: " << e.what() << endl;
    }

    ImGui_ImplSDL2_InitForSDLRenderer(window.data(), renderer.data());
    ImGui_ImplSDLRenderer2_Init(renderer.data());

    children.push_back(std::make_unique<JoystickListWindow>());
    children.push_back(std::make_unique<GameControllerListWindow>());


    // unsigned n_sensors = sdl::sensor::get_num_devices();
    // cout << "num sensors: " << n_sensors << endl;
}


App::~App()
{
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();

    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}


int
App::run()
{
    try {
        running = true;
        while (running) {
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


void
App::draw()
{
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // gui code goes here
    process_ui();

    ImGui::EndFrame();
    ImGui::Render();

    renderer.set_color(0x101010_rgb);
    // renderer.set_color(sdl::color::black);
    renderer.clear();

    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer.data());

    renderer.present();
}


void
App::process_ui()
{
    for (auto& c : children)
        c->process();
}


void
App::process_events()
{
    sdl::events::event event;
    while (sdl::events::poll(event)) {

        ImGui_ImplSDL2_ProcessEvent(&event);

        for (auto& c : children)
            c->handle(event);

        switch (sdl::events::type{event.type}) {
            using enum sdl::events::type;

            case quit:
                running = false;
                break;

            case controller_device_added:
                controllers.emplace_back(event.cdevice.which);
                break;

            case controller_device_removed:
                std::erase_if(controllers,
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
