#pragma once

#include <string>

void Init_SDL_and_GL();
void Quit_SDL_and_GL();


unsigned int LoadShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath);