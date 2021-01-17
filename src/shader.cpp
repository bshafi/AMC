#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL_ttf/SDL_ttf.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>

#include "gl_helper.hpp"
#include "shader.hpp"

Shader::Shader(const std::string &vertex, const std::string &fragment) {
    this->shader_program = LoadShaderProgram(vertex, fragment);
    assert(this->shader_program != 0);
}
Shader::~Shader() {
    glDeleteProgram(shader_program);
}

Shader::Shader(Shader &&other) {
    if (this != &other) {
        glDeleteShader(this->shader_program);
        this->shader_program = other.shader_program;

        other.shader_program = 0;
    }
    assert(this->shader_program != 0);
}

void Shader::use() {
    assert(this->shader_program != 0);
    glUseProgram(shader_program);
}


void Shader::bind_texture_to_sampler_2D(const std::vector<std::pair<std::string, std::reference_wrapper<Texture>>> &things) {
    // FIXME: Remove the magic number 16
    assert(things.size() <= 16);
    unsigned i = 0;

    for (const auto &[name, texture] : things) {
        glActiveTexture(GL_TEXTURE0 + i);
        texture.get().bind();
        int loc = glGetUniformLocation(this->shader_program, name.c_str());
        assert(loc != -1);
        glUniform1i(loc, i);
        ++i;
    }
}

Texture::Texture(const std::string &path) {
    this->id = LoadImage(path);
    assert(this->id != 0);
}
Texture::~Texture() {
    glDeleteTextures(1, &this->id);
}

Texture::Texture(Texture &&other) {
    if (this != &other) {
        glDeleteTextures(1, &this->id);
        this->id = other.id;
        other.id = 0;
    }

    assert(this->id != 0);
}

void Texture::bind() const {
    glBindTexture(GL_TEXTURE_2D, this->id);
}


unsigned Texture::get_id() const {
    return this->id;
}

void GLFunctionsWrapper::setFloat(int loc, const float &flt) {
    glUniform1f(loc, flt);
}

void GLFunctionsWrapper::setvec3(int loc, const glm::vec3 &v) {
    glUniform3f(loc, v.x, v.y, v.z);
}
void GLFunctionsWrapper::setvec2(int loc, const glm::vec2 &v) {
    glUniform2f(loc, v.x, v.y);
}

void GLFunctionsWrapper::setuvec2(int loc, const glm::uvec2 &v) {
    glUniform2ui(loc, v.x, v.y);
}
int GLFunctionsWrapper::getUniformLocation(int shader_program, const std::string &name) {
    return glGetUniformLocation(shader_program, name.c_str());
}

void GLFunctionsWrapper::setivec3(int loc, const glm::ivec3 &v) {
    glUniform3i(loc, v.x, v.y, v.z);
}

void GLFunctionsWrapper::setmat4x4(int loc, const glm::mat4x4 &m) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}