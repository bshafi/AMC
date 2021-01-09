#pragma once

#include <string>
#include <cstdint>
#include <glm/glm.hpp>

void Init_SDL_and_GL();
void Quit_SDL_and_GL();


unsigned int LoadShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath);
uint32_t LoadImage(const std::string &imagePath);
/*
template <typename T, typename ...Args>
constexpr size_t glstd140_size() {
    return glstd140_size<T>() + glstd140_size<Args...>();
}

template <>
constexpr size_t glstd140_size<GLint>() {
    return sizeof(GLint);
}

template <>
constexpr size_t glstd140_size<GLfloat>() {
    return sizeof(GLint);
}

template <>
constexpr size_t glstd140_size<GLuint>() {
    return sizeof(GLint);
}

template <>
constexpr size_t glstd140_size<glm::vec2>() {

}

template <typename ...Args>
struct glstd140_tuple {
    char c[glstd140_size<Args...>()];
};
*/