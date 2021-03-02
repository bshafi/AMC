#pragma once

#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL_ttf/SDL_ttf.h>

#include "standard.hpp"

void Init_SDL_and_GL();
void Quit_SDL_and_GL();

// Prints an error message if opengl had an error
// returns true if an error occurred 
bool glBreakOnError();

// Using a macro places the assert in the line where the error is found
// so the assert will print out a usefull line number
#ifndef NDEBUG
#define ASSERT_ON_GL_ERROR() if (glBreakOnError()) { assert(false); } do {} while (false)
#else
    #define ASSERT_ON_GL_ERROR() do {} while (false)
#endif

union SDL_Event;

// do not modify
// the event handle that gives the real window size event.user.data1xevent.user.data2
// both data1 and data2 are of type intptr_t
extern uint32_t WINDOW_TRUE_RESIZE_EVENT;
extern uint32_t SCENE_CHANGE_EVENT;


enum class GameState {
    TitleScreen,
    GamePlay,
    SaveSelect
};

using Primitive = std::variant<int>;

struct SceneChangeData {
    GameState next_game_state;
    std::optional<int> save_index;
};


void PushWindowTrueResizeEvent(int witdth, int height);

glm::uvec2 GetWindowBoundsFromTrueResizeEvent(const SDL_Event &event);
glm::uvec2 GetTrueWindowSize();

// user.event.data1 is a SceneChangeData pointer which must only be freed by the main thread
void PushSceneChangeEvent(const SceneChangeData &);

unsigned int LoadShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath);
uint32_t LoadImage(const std::string &imagePath, uint32_t *width, uint32_t *height);
uint32_t RasterizeText(const std::string &text);
