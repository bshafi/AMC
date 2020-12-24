#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl_glext.h>
#include <SDL2/SDL_image.h>
#include <SDL_ttf/SDL_ttf.h>

#include <glm/glm.hpp>
#include <array>

#include <cassert>
#include <cstdint>
#include <iostream>
#include <fstream>

#include <type_traits>

#include "gl_helper.hpp"

constexpr uint32_t DEFAULT_SDL_INIT_FLAGS = SDL_INIT_EVERYTHING;
constexpr uint32_t DEFAULT_IMG_INIT_FLAGS = IMG_INIT_PNG;

void Init_SDL_and_GL() {
    assert(SDL_Init(DEFAULT_SDL_INIT_FLAGS) == 0);
    //assert(IMG_Init(DEFAULT_IMG_INIT_FLAGS) == DEFAULT_IMG_INIT_FLAGS);

    assert(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) == 0);
    assert(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2) == 0);
    assert(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) == 0);
    assert(SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG) == 0);

    //SDL_SetHint(SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG, "1");
    assert(SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "1") == SDL_TRUE);
}
void Quit_SDL_and_GL() {
    //IMG_Quit();
    SDL_Quit();
}


unsigned int LoadShaderProgram(const std::string &vertexShaderPath, const std::string &fragmentShaderPath) {
    std::ifstream vertexShaderFile(vertexShaderPath), 
                  fragmentShaderFile(fragmentShaderPath);
    if (vertexShaderFile.bad() || !vertexShaderFile.is_open()) {
        if (fragmentShaderFile.bad() || !fragmentShaderFile.is_open()) {
            throw std::invalid_argument("Could not find files at path " + vertexShaderPath + " or " + fragmentShaderPath);
        } else {
            throw std::invalid_argument("Could not find file at path " + vertexShaderPath);
        }
    }
    if (fragmentShaderFile.bad() || !fragmentShaderFile.is_open()) {
        throw std::invalid_argument("Cound not find file at path " + fragmentShaderPath);
    }

    // Reads the entire file to string
    std::string vertexShaderStr = std::string(std::istreambuf_iterator<char>(vertexShaderFile), std::istreambuf_iterator<char>());
    std::string fragmentShaderStr = std::string(std::istreambuf_iterator<char>(fragmentShaderFile), std::istreambuf_iterator<char>());

    const char *vertexShaderSource = vertexShaderStr.c_str();
    const char *fragmentShaderSource = fragmentShaderStr.c_str();

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int  success;
    std::string infoLog;
    infoLog.resize(512);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog.data());
        std::cout << "[VERTEX::COMPILATION_FAILED]" << infoLog << std::endl;
    }

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog.data());
        std::cout << "[FRAGMENT::COMPILATION_FAILED]" << infoLog << std::endl;;
    }

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog.data());
        
        std::cout << "[PROGRAM::COMPILATION_FAILED]" << infoLog << std::endl;;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}


// Assume that the gl types are the same as the cpp types
static_assert(std::is_same<uint32_t, GLuint>::value);
static_assert(std::is_same<int32_t, GLint>::value);
static_assert(std::is_same<float, GLfloat>::value);

// Assume the glm types are tightly packed
static_assert(sizeof(glm::vec3) == sizeof(std::array<float, 3>));
static_assert(sizeof(glm::vec2) == sizeof(std::array<float, 2>));
static_assert(sizeof(glm::ivec2) == sizeof(std::array<int32_t, 2>));
static_assert(sizeof(glm::ivec3) == sizeof(std::array<int32_t, 3>));
static_assert(sizeof(glm::uvec2) == sizeof(std::array<uint32_t, 2>));
static_assert(sizeof(glm::uvec3) == sizeof(std::array<uint32_t, 3>));