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

#include "JoystickListWindow.hpp"

using std::filesystem::path;
using std::cout;
using std::cerr;
using std::endl;

using namespace sdl::literals;

path assets_path = "assets";


ImVector<ImWchar> glyph_ranges;


App::App() :
    sdl_init{ sdl::init::flag::video | sdl::init::flag::joystick },
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


    ImGui_ImplSDL2_InitForSDLRenderer(window.data(), renderer.data());
    ImGui_ImplSDLRenderer2_Init(renderer.data());

    children.push_back(std::make_unique<JoystickListWindow>());
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

            case SDL_QUIT:
                running = false;
                break;

        }

    }
}
