#include <iostream>
#include <vector>

#include "gl_helper.hpp"
#include "shader.hpp"

Shader::Shader(const std::string &vertex, const std::string &fragment) {
    this->shader_program = LoadShaderProgram(vertex, fragment);
    ASSERT_ON_GL_ERROR();
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
    ASSERT_ON_GL_ERROR();

    glUseProgram(shader_program);

    ASSERT_ON_GL_ERROR();
}


void Shader::bind_texture_to_sampler_2D(const std::vector<std::pair<std::string, std::reference_wrapper<Texture>>> &things) {
    // TODO: Remove the magic number 16
    assert(things.size() <= 16);
    unsigned i = 0;
    
    ASSERT_ON_GL_ERROR();

    for (const auto &[name, texture] : things) {
        this->use();
        int loc = glGetUniformLocation(this->shader_program, name.c_str());
        assert(loc != -1);
        glUniform1i(loc, i);

        // If glUseShader is called before before this function this function doesn't work
        glUseProgram(0); // Reseting the shader program

        glActiveTexture(GL_TEXTURE0 + i);
        texture.get().bind();
        ++i;

        ASSERT_ON_GL_ERROR();
    }

    ASSERT_ON_GL_ERROR();
}

void Shader::bind_UBO(const std::string &ubo_name, unsigned int loc) {
    auto index = glGetUniformBlockIndex(this->shader_program, ubo_name.c_str());
    glUniformBlockBinding(this->shader_program, index, loc);

    ASSERT_ON_GL_ERROR();
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


void GLFunctionsWrapper::setuint(int loc, const uint32_t &i) {
    glUniform1ui(loc, i);
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
    int ret = glGetUniformLocation(shader_program, name.c_str());
    ASSERT_ON_GL_ERROR();

    return ret;
}

void GLFunctionsWrapper::setivec3(int loc, const glm::ivec3 &v) {
    glUniform3i(loc, v.x, v.y, v.z);
}

void GLFunctionsWrapper::setmat4x4(int loc, const glm::mat4x4 &m) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}

