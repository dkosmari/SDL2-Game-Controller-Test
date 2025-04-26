#include <exception>
#include <iostream>
#include <string>
#include <filesystem>

#include <sdl2xx/sdl.hpp>
#include <sdl2xx/ttf.hpp>

#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

using std::cout;
using std::cerr;
using std::endl;
using std::filesystem::path;

using sdl::vec2;

using namespace sdl::literals;


path assets_path = "assets";


struct App {

    sdl::init sdl_init{ sdl::init::flag::video | sdl::init::flag::game_controller };
    // sdl::ttf::init ttf_init;

    sdl::window window{
        PACKAGE_NAME,
        sdl::window::pos_undefined,
        {1280, 720},
        sdl::window::flag::resizable
    };

    sdl::renderer renderer{
        window,
        -1,
        sdl::renderer::flag::accelerated | sdl::renderer::flag::present_vsync
    };

    // sdl::ttf::font main_font{assets_path / "LiberationSans-Regular.ttf", 24};

    bool running = false;


    App()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        io.Fonts->AddFontFromFileTTF((assets_path / "LiberationSans-Regular.ttf").c_str(),
                                     20, nullptr, nullptr);

        ImGui_ImplSDL2_InitForSDLRenderer(window.data(), renderer.data());
        ImGui_ImplSDLRenderer2_Init(renderer.data());
    }


    ~App()
    {
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }


    int
    run()
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
    draw()
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
    process_ui()
    {
        ImGui::Begin("Test");

        ImGui::Text("Here's some text");

        if (ImGui::Button("Click me")) {
            cout << "clicked" << endl;
        }

        ImGui::End();
    }


    void
    process_events()
    {
        while (auto event = sdl::events::poll()) {

            ImGui_ImplSDL2_ProcessEvent(&*event);

            switch (event->type) {

                case SDL_QUIT:
                    running = false;
                    break;

            }

        }
    }
};


int main(int, char* [])
{
    try {
        App app;
        return app.run();
    }
    catch (std::exception& e) {
        cerr << "ERROR: " << e.what() << endl;
        return -1;
    }
}
