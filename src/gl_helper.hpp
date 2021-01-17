#pragma once

#include <string>
#include <cstdint>
#include <glm/glm.hpp>

void Init_SDL_and_GL();
void Quit_SDL_and_GL();


unsigned int LoadShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath);
uint32_t LoadImage(const std::string &imagePath);
