#include <cassert>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <map>

#include "gl_helper.hpp"
#include "hello_cube.hpp"
#include "camera.hpp"
#include "world.hpp"
#include "chunk.hpp"
#include "gui.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"

int main(const int, const char**) {
    SDL_Window *window = Init_SDL_and_GL();

    assert(window);

    int width, height;
    SDL_GL_GetDrawableSize(window, &width, &height);
    glViewport(0, 0, width, height);

    GameState state = GameState::TitleScreen;
    World world;
    world.load("saves/save0.hex");

    ASSERT_ON_GL_ERROR();
    
    GUI gui;
    {
        auto *title_background = new Texture("resources/title_image.png");
        ADD_POINTER(title_background);
        auto *title_text = new Texture("resources/title.png");
        ADD_POINTER(title_text);
        auto *play_button = new Texture("resources/play_button.png");
        ADD_POINTER(play_button);
        auto *title_text_sprite = new GUIImage(*title_text);
        ADD_POINTER(title_text_sprite);
        const auto play_button_rect = frect{ 0, 0, static_cast<float>(play_button->width()), static_cast<float>(play_button->height()) };
        const auto play_button_normal = frect{ 0, 0, play_button_rect.w, play_button_rect.h / 2 };
        const auto play_button_hover = frect{ 0, play_button_rect.h / 2, play_button_rect.w, play_button_rect.h / 2 };
        auto *play_button_sprite = new Button(*play_button, play_button_normal, play_button_hover);
        ADD_POINTER(play_button_sprite);
        auto *title_background_sprite = new GUIImage(*title_background, true);
        ADD_POINTER(title_background_sprite);
        auto *things = new HBisection(GUI::Padding(title_text_sprite, 0.1), GUI::VPadding(play_button_sprite, 0.2));
        ADD_POINTER(things);
        GUIElement *root = new ZStack(std::vector<GUIElement*>{ title_background_sprite, things });
        ADD_POINTER(root);
        std::vector<Texture*> texs = { title_text, play_button, title_background };
        gui.set_textures(texs);
        gui.set_root(&root);
    }

    ASSERT_ON_GL_ERROR();

    uint32_t ticks = SDL_GetTicks();
    bool is_running = true;
    std::vector<SDL_Event> events;
    while (is_running) {
        events.clear();
        // TODO: Filter mouse events and window resize events through here
        for (SDL_Event event = {}; SDL_PollEvent(&event);) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch (event.type) {
            case SDL_QUIT: 
                is_running = false;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.scancode == SDL_SCANCODE_Q) {
                    
                }
                break;
            default:
                if (event.type == SCENE_CHANGE_EVENT) {
                    SceneChangeData *scd = static_cast<SceneChangeData*>(event.user.data1);
                    assert(scd);

                    delete scd;
                }
            break;
            }
            events.push_back(event);
        }
        switch (state) {
        case GameState::TitleScreen:
            gui.handle_events(events);
            if (gui.is_clicked(0)) {
                state = GameState::GamePlay;
            }
            break;
        case GameState::SaveSelect:
            assert(false);
            break;
        case GameState::GamePlay:
            world.handle_events(events);
            break;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        ASSERT_ON_GL_ERROR();

        {
            ImGui::Begin("Configurations");
            if (ImGui::Button("Toggle Debug Mode")) {
                world.player.toggle_debug_mode(world);
            }
            ImGui::DragFloat("gravity", &world.player.gravity, 0.1f, 1.0f, 8.0f);
            ImGui::End();
        }

        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        switch (state) {
        case GameState::TitleScreen:
            gui.draw();
            break;
        case GameState::SaveSelect:
            assert(false);
            break;
        case GameState::GamePlay:
            world.draw();
            break;
        }

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);

        uint32_t delta_ticks = SDL_GetTicks() - ticks;
        if (delta_ticks * FPS  < 1000) {
            SDL_Delay(1000 / FPS - delta_ticks);
        }
        ticks = SDL_GetTicks();
    }

    SDL_DestroyWindow(window);

    Quit_SDL_and_GL();
#ifdef MANUAL_LEAK_CHECK
    // After all the destructors are called this function will run
    atexit([](){
        std::cout << "pointers size: " << pointers.size() << "\n";
        for (auto &[p, loc] : pointers) {
            auto &[file_name, line] = loc;
            std::cout << file_name << ": " << line << " Not freed" << std::endl;
        }
    });
#endif
}