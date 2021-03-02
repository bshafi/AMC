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

constexpr int INITIAL_WINDOW_WIDTH = 853;
constexpr int INITIAL_WINDOW_HEIGHT = 480;
constexpr uint32_t DEFAULT_SDL_WINDOW_FLAGS = SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
constexpr uint32_t FPS = 30;

int main(const int, const char**) {
    Init_SDL_and_GL();

    SDL_Window *window = SDL_CreateWindow(
        "Another Minecraft Clone", 
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 
        INITIAL_WINDOW_WIDTH,
        INITIAL_WINDOW_HEIGHT, 
        DEFAULT_SDL_WINDOW_FLAGS
    );
    assert(window);
    
    SDL_GLContext sdl_glcontext = SDL_GL_CreateContext(window);
    assert(sdl_glcontext);

    assert(glewInit() == GLEW_OK);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(135 / 255.f, 206 / 255.f,235 / 255.f, 1.0f);

    int width, height;
    SDL_GL_GetDrawableSize(window, &width, &height);
    glViewport(0, 0, width, height);

    Renderer renderer;

    ASSERT_ON_GL_ERROR();

    GUI gui;
    {
        auto *title_background = new Texture("resources/title_image.png");
        auto *title_text = new Texture("resources/title.png");
        auto *play_button = new Texture("resources/play_button.png");
        
        auto *title_text_sprite = new GUIElement(Sprite{ title_text });
        const auto play_button_rect = frect{ 0, 0, static_cast<float>(play_button->width()), static_cast<float>(play_button->height()) };
        const auto play_button_normal = frect{ 0, 0, play_button_rect.w, play_button_rect.h / 2 };
        const auto play_button_hover = frect{ 0, play_button_rect.h / 2, play_button_rect.w, play_button_rect.h / 2 };
        auto *play_button_sprite = new GUIElement(Button{ play_button, play_button_normal, play_button_hover });
        auto *title_background_sprite = new GUIElement(Sprite{ title_background, true });
        //auto *sprite = new GUIElement(HBisection{ sprite1, new GUIElement(VBisection{ nullptr, new GUIElement(VBisection{ sprite2, nullptr, 8.f/9.f }), 0.1f }) });
        auto *things = new GUIElement(HBisection{ GUI::VPadding(title_text_sprite, 0.3f), GUI::VPadding(play_button_sprite, 2/10.f) });
        auto *root = new GUIElement(std::vector<GUIElement*>{ title_background_sprite, things });
        //auto *sprite = GUI::HPadding(sprite1, 0.1f);
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
        for (SDL_Event event = {}; SDL_PollEvent(&event);) {
            switch (event.type) {
            case SDL_QUIT: is_running = false; break;
            case SDL_WINDOWEVENT: {
                switch (event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    int w, h;
                    SDL_GL_GetDrawableSize(window, &w, &h);
                    PushWindowTrueResizeEvent(w, h);
                    // glViewport(0, 0, w, h);
                    // don't handle the resize here, handle it in WINDOW_TRUE_RESIZE_EVENT
                    break;
                default:
                    break;
                }
            }
            case SDL_KEYDOWN:
                if (event.key.keysym.scancode == SDL_SCANCODE_Q) {
                    
                }
                break;
            default:
                if (event.type == WINDOW_TRUE_RESIZE_EVENT) {
                    intptr_t w = reinterpret_cast<intptr_t>(event.user.data1), h = reinterpret_cast<intptr_t>(event.user.data2);
                    glViewport(0, 0, static_cast<int>(w), static_cast<int>(h));
                } else if (event.type == SCENE_CHANGE_EVENT) {
                    SceneChangeData *scd = static_cast<SceneChangeData*>(event.user.data1);
                    assert(scd);

                    delete scd;
                }
            break;
            }
            events.push_back(event);
        }
        gui.handle_events(events);

        ASSERT_ON_GL_ERROR();


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        gui.draw(renderer);

        SDL_GL_SwapWindow(window);

        uint32_t delta_ticks = SDL_GetTicks() - ticks;
        if (delta_ticks * FPS  < 1000) {
            SDL_Delay(1000 / FPS - delta_ticks);
        }
        ticks = SDL_GetTicks();

    }

    SDL_GL_DeleteContext(sdl_glcontext);
    SDL_DestroyWindow(window);

    Quit_SDL_and_GL();
}