#include <cassert>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <map>

#include "gl_helper.hpp"
#include "hello_cube.hpp"
#include "camera.hpp"
#include "gui.hpp"
#include "world.hpp"
#include "chunk.hpp"

constexpr int INITIAL_WINDOW_WIDTH = 640;
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
    glClearColor(0.3f, 0.f, 0.f, 1.0f);

    int width, height;
    SDL_GL_GetDrawableSize(window, &width, &height);
    glViewport(0, 0, width, height);

    GameState game_state = GameState::TitleScreen;

    ASSERT_ON_GL_ERROR();

    GUI title_screen;
    title_screen.load_images({
        { "resources/title_image.png",  Image{ { -1, -1, 2, 2     }, { 0, 0, 1, 1 }, 2 } },
        { "resources/title.png",        Image{ { -.5, .4, 1, .4   }, { 0, 0, 1, 1 }, 1 } },
        { "resources/play.png", Button{ Image{ { -.1, -.9, .2, .1 }, { 0, 0, 1, 1 }, 0 }, 0 }}
    });
    GUI save_select_screen;
    save_select_screen.load_images({
        { "resources/title_image.png",   Image{ {  -1, -1, 2, 2     }, { 0, 0, 1, 1 }, 2 } },
        { "resources/save0.png", Button{ Image{ {-0.5, 0.3, 1, 0.5  }, {0, 0, 1, 1}},  0} },
        { "resources/save1.png", Button{ Image{ {-0.5, -0.2, 1, 0.5 }, {0, 0, 1, 1}}, 1} },
        { "resources/save2.png", Button{ Image{ {-0.5, -0.7, 1, 0.5 }, {0, 0, 1, 1}}, 2} }
    });

    World world;

    ASSERT_ON_GL_ERROR();

    world.camera.pos(glm::vec3{6, 111, 29 });
    world.camera.pitch(-7.04345);
    world.camera.yaw(0.43354);


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
                    std::cout << world.camera.pos().x << ", " << world.camera.pos().y << ", "  << world.camera.pos().z << std::endl;
                    std::cout << world.camera.pitch() << ", " << world.camera.yaw() << std::endl;
                }
                break;
            default:
                if (event.type == WINDOW_TRUE_RESIZE_EVENT) {
                    intptr_t w = reinterpret_cast<intptr_t>(event.user.data1), h = reinterpret_cast<intptr_t>(event.user.data2);
                    glViewport(0, 0, static_cast<int>(w), static_cast<int>(h));
                } else if (event.type == SCENE_CHANGE_EVENT) {
                    SceneChangeData *scd = static_cast<SceneChangeData*>(event.user.data1);
                    assert(scd);

                    switch (scd->next_game_state) {
                    case GameState::TitleScreen:
                        game_state = GameState::TitleScreen;
                        break;
                    case GameState::SaveSelect:
                        game_state = GameState::SaveSelect;
                        break;
                    case GameState::GamePlay:
                        assert(scd->save_index.has_value());
                        world.load("saves/save" + std::to_string(scd->save_index.value()) + ".hex");
                        game_state = GameState::GamePlay;
                        break;
                    default:
                        assert(false);
                    }

                    delete scd;
                }
            break;
            }
            events.push_back(event);
        }

        switch (game_state) {
        case GameState::TitleScreen:
            title_screen.handle_events(events);
            if (title_screen.is_clicked(0)) {
                game_state = GameState::SaveSelect;
            }
            break;
        case GameState::GamePlay:
            world.handle_events(events);
            break;
        case GameState::SaveSelect:
            save_select_screen.handle_events(events);
            for (int i = 0; i < 3; ++i) {
                if (save_select_screen.is_clicked(i)) {
                    PushSceneChangeEvent(SceneChangeData{ GameState::GamePlay, i });
                }
            }
            break;
        }
        ASSERT_ON_GL_ERROR();


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        switch (game_state) {
        case GameState::TitleScreen:
            title_screen.draw();
            break;
        case GameState::GamePlay:
            world.draw();
            break;
        case GameState::SaveSelect:
            save_select_screen.draw();
            break;
        }

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