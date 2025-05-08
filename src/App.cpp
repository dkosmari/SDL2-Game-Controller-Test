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

path assets_path = "assets";


ImVector<ImWchar> glyph_ranges;


App::App() :
    sdl_init{
        sdl::init::flag::video,
        sdl::init::flag::joystick,
        sdl::init::flag::game_controller,
        sdl::init::flag::sensor,
        sdl::init::flag::haptic
    },
    window{
        PACKAGE_NAME,
        sdl::window::pos_undefined,
        {1600, 900},
        sdl::window::flag::resizable
    },
    renderer{
        window,
        -1,
        sdl::renderer::flag::accelerated | sdl::renderer::flag::present_vsync
    }
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
    builder.AddText("•←↑→↓↖↗↘↙");
    builder.BuildRanges(&glyph_ranges);

    io.Fonts->AddFontFromFileTTF((assets_path / "DejaVuSans.ttf").c_str(),
                                 22,
                                 nullptr,
                                 glyph_ranges.Data);


    sdl::game_controller::try_add_mappings("gamecontrollerdb.txt");
    try {
        for (auto entry : std::filesystem::directory_iterator{"mappings"}) {
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

    // renderer.set_color(0x101010_rgb);
    renderer.set_color(sdl::color::black);
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
    while (auto event = sdl::events::poll()) {

        ImGui_ImplSDL2_ProcessEvent(&*event);

        for (auto& c : children)
            c->handle(*event);

        switch (event->type) {

            case sdl::events::type::e_quit:
                running = false;
                break;

        }

    }
}
