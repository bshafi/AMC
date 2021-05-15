#include <iostream>
#include <vector>

#include "gl_helper.hpp"
#include "shader.hpp"


Binding::Binding(const std::string &name, const Texture &tex)
    : name{ name }, id{ tex.get_id() }, target{ GL_TEXTURE_2D } {
}
Binding::Binding(const std::string &name, const CubeMap &cube_map)
    : name{ name }, id{ cube_map.get_id() }, target{ GL_TEXTURE_CUBE_MAP } {
}

Shader::Shader(const std::string &vertex, const std::string &fragment, const std::string &geometry) {
    this->shader_program = LoadShaderProgram(vertex, fragment, geometry);
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


void Shader::apply_bindings(const std::vector<Binding> &bindings) {
    // TODO: Remove the magic number 16
    assert(bindings.size() <= 16);
    unsigned i = 0;
    
    ASSERT_ON_GL_ERROR();

    for (const auto &[name, id, target] : bindings) {
        this->use();
        int loc = glGetUniformLocation(this->shader_program, name.c_str());
        assert(loc != -1);
        glUniform1i(loc, i);

        // If glUseShader is called before before this function this function doesn't work
        glUseProgram(0); // Reseting the shader program

        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(target, id);
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
    this->id = LoadImage(path, &m_width, &m_height);
    assert(this->id != 0);
    assert(m_width != 0);
    assert(m_height != 0);
}
Texture::~Texture() {
    glDeleteTextures(1, &this->id);
}

Texture& Texture::operator=(Texture rhs) {
    swap(*this, rhs);
    return *this;
}
Texture::Texture(Texture &&other)
    : id{ 0 }, m_width{ 0 }, m_height{ 0 } {
    swap(*this, other);
}
void swap(Texture &rhs, Texture &lhs) {
    using std::swap;

    swap(rhs.id, lhs.id);
    swap(rhs.m_height, lhs.m_height);
    swap(rhs.m_width, lhs.m_width);
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


CubeMap::CubeMap(const std::string &right, const std::string &left, const std::string &top, const std::string &bottom, const std::string &back, const std::string &front) {
    this->id = LoadCubeMap(right, left, back, bottom, back, front);
    assert(this->id != 0);
}
CubeMap::~CubeMap() {
    glDeleteTextures(1, &id);
}

CubeMap& CubeMap::operator=(CubeMap rhs) {
    swap(*this, rhs);
    return *this;
}
CubeMap::CubeMap(CubeMap &&other)
    : id{ 0 } {
    swap(*this, other);
}
uint32_t CubeMap::get_id() const {
    return id;
}
void swap(CubeMap &lhs, CubeMap &rhs) {
    using std::swap;

    swap(lhs.id, rhs.id);
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

