#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl_glext.h>
#include <SDL2/SDL_image.h>
#include <SDL_ttf/SDL_ttf.h>

#include <cassert>
#include <stdint.h>
#include <iostream>

#include "gl_helper.hpp"
#include "hello_cube.hpp"

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

    auto glewError = glewInit();
    if (glewError != GLEW_OK) {
        printf("%s\n", glewGetErrorString(glewError));
    }

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    int width, height;
    SDL_GL_GetDrawableSize(window, &width, &height);
    glViewport(0, 0, width, height);

    
    //unsigned int shader_program = LoadShaderProgram("shaders/hello_cube.vert", "shaders/hello_cube.frag");

    HelloCube hc;

    uint32_t ticks = SDL_GetTicks();
    float delta_time_s = 0.0f;

    bool is_running = true;
    while (is_running) {
        for (SDL_Event event = {}; SDL_PollEvent(&event);) {
            switch (event.type) {
            case SDL_QUIT:
                is_running = false;
                break;
            }
        }
        hc.update();

        glClear(GL_COLOR_BUFFER_BIT);

        hc.draw();

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