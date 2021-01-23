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

unsigned int LoadShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath);
uint32_t LoadImage(const std::string &imagePath);
uint32_t RasterizeText(const std::string &text);