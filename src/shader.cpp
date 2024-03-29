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

Shader& Shader::operator=(Shader rhs) {
    swap(*this, rhs);
    return *this;
}
Shader::Shader(Shader &&shader)
    : shader_program{ 0 } {
    swap(*this, shader);
}
void swap(Shader &rhs, Shader &lhs) {
    using std::swap;

    swap(rhs.shader_program, lhs.shader_program);
}


void Shader::use() {
    assert(this->shader_program != 0);
    ASSERT_ON_GL_ERROR();

    glUseProgram(shader_program);

    ASSERT_ON_GL_ERROR();
}


void Shader::bind_texture_to_sampler_2D(const std::vector<std::pair<std::string, std::reference_wrapper<const Texture>>> &things) {
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
    ASSERT_ON_GL_ERROR();
    auto index = glGetUniformBlockIndex(this->shader_program, ubo_name.c_str());
    glUniformBlockBinding(this->shader_program, index, loc);

    ASSERT_ON_GL_ERROR();
}

Texture::Texture(const std::string &path) {
    this->id = LoadImage(path, &m_width, &m_height);
    assert(this->id != 0);
    assert(m_width != 0);
    assert(m_height != 0);
}
Texture::~Texture() {
    glDeleteTextures(1, &this->id);
}

Texture::Texture(Texture &&other) {
    if (this != &other) {
        glDeleteTextures(1, &this->id);
        this->id = other.id;
        this->m_height = other.m_height;
        this->m_width = other.m_width;

        other.id = 0;
        other.m_height = 0;
        other.m_width = 0;
    }

    assert(this->id != 0);
    assert(m_width != 0);
    assert(m_height != 0);
}

void Texture::bind() const {
    glBindTexture(GL_TEXTURE_2D, this->id);
}

uint32_t Texture::width() const {
    return m_width;
}
uint32_t Texture::height() const {
    return m_height;
}


unsigned Texture::get_id() const {
    return this->id;
}

frect Texture::rect() const {
    return frect{
        0,
        0,
        static_cast<float>(width()),
        static_cast<float>(height())
    };
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
void GLFunctionsWrapper::setvec4(int loc, const glm::vec4 &v) {
    glUniform4f(loc, v.x, v.y, v.z, v.w);
}

void GLFunctionsWrapper::setuvec2(int loc, const glm::uvec2 &v) {
    glUniform2ui(loc, v.x, v.y);
}
int GLFunctionsWrapper::getUniformLocation(int shader_program, const std::string &name) {
    int ret = glGetUniformLocation(shader_program, name.c_str());
    ASSERT_ON_GL_ERROR();

    return ret;
}
void GLFunctionsWrapper::setivec2(int loc, const glm::ivec2 &v) {
    glUniform2i(loc, v.x, v.y);
}

void GLFunctionsWrapper::setivec3(int loc, const glm::ivec3 &v) {
    glUniform3i(loc, v.x, v.y, v.z);
}

void GLFunctionsWrapper::setmat4x4(int loc, const glm::mat4x4 &m) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}



uint32_t generate_ubo(Shader &shader, glm::mat4 view) {
    ASSERT_ON_GL_ERROR();

    uint32_t globals_3d_ubo;
    glGenBuffers(1, &globals_3d_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, globals_3d_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, nullptr, GL_STATIC_DRAW);

    glm::uvec2 win_size = GetTrueWindowSize();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
    glm::mat4 projection = glm::perspective(static_cast<float>(M_PI / 4), static_cast<float>(win_size.x) / static_cast<float>(win_size.y), 0.1f, 100.f);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));

    shader.bind_UBO("globals_3d", 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, globals_3d_ubo);

    ASSERT_ON_GL_ERROR();

    return globals_3d_ubo;
}
void update_ubo_matrices(uint32_t globals_3d_ubo, glm::mat4 view) {
    glBindBuffer(GL_UNIFORM_BUFFER, globals_3d_ubo);

    glm::uvec2 win_size = GetTrueWindowSize();
    glm::mat4 projection = glm::perspective(static_cast<float>(M_PI / 4), static_cast<float>(win_size.x) / static_cast<float>(win_size.y), 0.1f, 100.f);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}