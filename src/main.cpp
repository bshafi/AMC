#include <cassert>
#include <stdint.h>
#include <iostream>
#include <vector>

#include "gl_helper.hpp"
#include "hello_cube.hpp"
#include "chunk.hpp"
#include "camera.hpp"

constexpr int INITIAL_WINDOW_WIDTH = 640;
constexpr int INITIAL_WINDOW_HEIGHT = 480;
constexpr uint32_t DEFAULT_SDL_WINDOW_FLAGS = SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
constexpr uint32_t FPS = 60;


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

    {
        GLenum glewError = glewInit();
        if (glewError != GLEW_OK) {
            printf("%s\n", glewGetErrorString(glewError));
        }
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    int width, height;
    SDL_GL_GetDrawableSize(window, &width, &height);
    glViewport(0, 0, width, height);

    uint32_t ticks = SDL_GetTicks();

    Camera camera;
    Chunk ck;

    unsigned globals_3d_ubo;
    {
        glGenBuffers(1, &globals_3d_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, globals_3d_ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, nullptr, GL_DYNAMIC_DRAW);
        glm::mat4 view = camera.view_matrix();
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
        glm::mat4 projection = glm::perspective(static_cast<float>(M_PI / 4), 640.f / 480.f, 0.1f, 100.f);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));

        ck.shader.bind_UBO("globals_3d", 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, globals_3d_ubo);

        ASSERT_ON_GL_ERROR();
    }

    bool is_running = true;
    std::vector<SDL_Event> events;
    bool cursor_show = true;
    while (is_running) {
        events.clear();
        for (SDL_Event event = {}; SDL_PollEvent(&event);) {
            switch (event.type) {
            case SDL_QUIT: is_running = false; break;
            case SDL_WINDOWEVENT: {
                switch (event.window.type) {
                case SDL_WINDOWEVENT_RESIZED:
                    glViewport(0, 0, event.window.data1, event.window.data2);
                    break;
                }
            }
                break;
            default: break;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                SDL_SetRelativeMouseMode(SDL_TRUE);
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                SDL_SetRelativeMouseMode(SDL_FALSE);
            }
            if (event.type == SDL_MOUSEMOTION && SDL_GetRelativeMouseMode() == SDL_TRUE) {
                camera.rotate_right(M_PI * event.motion.xrel / 1000.0f);
                camera.rotate_upwards(-M_PI * event.motion.yrel / 1000.0f);
            }
            events.push_back(event);
        }
        {
            const auto keypresses = SDL_GetKeyboardState(NULL);
            if (keypresses[SDL_SCANCODE_A]) {
                camera.pos(camera.pos() - camera.right() * 0.1f);
            }
            if (keypresses[SDL_SCANCODE_D]) {
                camera.pos(camera.pos() + camera.right() * 0.1f);
            }
            if (keypresses[SDL_SCANCODE_S])  {
                camera.pos(camera.pos() - camera.forward() * 0.1f);
            }
            if (keypresses[SDL_SCANCODE_W])  {
                camera.pos(camera.pos() + camera.forward() * 0.1f);
            }
        }
        ASSERT_ON_GL_ERROR();


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        {
            glBindBuffer(GL_UNIFORM_BUFFER, globals_3d_ubo);
            glm::mat4 view = camera.view_matrix();
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
            ASSERT_ON_GL_ERROR();

            ck.draw();

            ASSERT_ON_GL_ERROR();
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