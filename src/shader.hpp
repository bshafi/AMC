#pragma once

#include <glm/glm.hpp>
#include <string>
#include <cassert>

/*
    The GLFunctions wrapper provides access to opengl functions such as glUniform1i without
    polluting the global namespace. These functions should only be used in the Uniform class.
 */
namespace GLFunctionsWrapper {
    void setuint(int loc, const uint32_t &i);
    void setFloat(int loc, const float &flt);
    void setvec3(int loc, const glm::vec3 &);
    void setvec2(int loc, const glm::vec2 &);
    void setuvec2(int loc, const glm::uvec2 &);
    void setivec3(int loc, const glm::ivec3 &);
    void setmat4x4(int loc, const glm::mat4x4 &);

    int getUniformLocation(int shader_program, const std::string &name);

    // Is used to make sure that the function is implemented for that uniform type
    template <typename T>
    struct shader_is_implemented {
        static const bool value = std::is_same<T, float>::value || std::is_same<T, glm::vec3>::value || std::is_same<T, glm::vec2>::value ||
                            std::is_same<T, glm::uvec2>::value || std::is_same<T, glm::ivec3>::value || std::is_same<T, glm::mat4x4>::value ||
                            std::is_same<T, uint32_t>::value;
    };

    // This forces the static_assert to evaluate on the type argument rather than evaluating
    // all the time
    // This allows the enforcement of certain types being implemented
    template <typename T>
    struct always_false {
        static const bool value = false;
    };
};

class Shader;

/*
    Allows acces to the value of uniform variables in a shader program
 */
template <typename T>
class Uniform {
public:
    // Only for use within the Shader class, DO NOT USE THIS OUTSIDE THE SHADER CLASS
    Uniform(int variable_data = 0) : loc{ variable_data} {
        assert(loc != -1);

        // If the uniform value isn't set immediately after the uniform location is gotten
        // OpenGl ignores all assignments to the uniforms values
        // The following code sets the value to the default constructor to avoid this problem
        set(T());
    }

    // TODO: Use the * operator and -> operator to simplify getting and setting
    void set(const T &val);
    T get() const;
private:
    int loc;
    T value;
};

struct Sampler2D;
class Texture;

class Shader {
public:
    Shader(const std::string &vertex, const std::string &fragment);
    Shader();
    ~Shader();

    Shader(const Shader&) = delete;
    Shader(Shader &&);

    // TODO: Make sure the type matches the correct uniform variable
    template <typename T>
    Uniform<T> retrieve_shader_variable(const std::string &variable_name) {
        static_assert(GLFunctionsWrapper::shader_is_implemented<T>::value, "You need to implement this type");
        assert(this->shader_program != 0);

        this->use();
    
        return Uniform<T>(GLFunctionsWrapper::getUniformLocation(shader_program, variable_name));

    }
    
    void use();

    void bind_texture_to_sampler_2D(const std::vector<std::pair<std::string, std::reference_wrapper<Texture>>> &bindings);

    // FIXME: This is a bit of a hack 
    void bind_UBO(const std::string &ubo_name, unsigned int loc);

    // Only for use within the shader.hpp file
    unsigned get_shader_program() const {
        return shader_program;
    }
private:
    unsigned shader_program;
};

struct Sampler2D {
    int active_texture_binding;
};


class Texture {
public:
    Texture(const std::string &path);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture(Texture &&other);

    void bind() const;

    unsigned get_id() const;
private:
    unsigned int id;
};


template <typename T>
void Uniform<T>::set(const T &val) {
    this->value = val; 
    if constexpr (std::is_same<T, uint32_t>::value) {
        GLFunctionsWrapper::setuint(loc, val);
    } else if constexpr(std::is_same<T, float>::value) {
        GLFunctionsWrapper::setFloat(loc, val);
    } else if constexpr(std::is_same<T, glm::vec1>::value) {
        GLFunctionsWrapper::setFloat(loc, val.x);
    } else if constexpr(std::is_same<T, glm::vec2>::value) {
        GLFunctionsWrapper::setvec2(loc, val);
    } else if constexpr(std::is_same<T, glm::vec3>::value) {
        GLFunctionsWrapper::setvec3(loc, val);
    } else if constexpr(std::is_same<T, glm::uvec2>::value) {
        GLFunctionsWrapper::setuvec2(loc, val);
    } else if constexpr(std::is_same<T, glm::ivec3>::value) {
        GLFunctionsWrapper::setivec3(loc, val);
    } else if constexpr(std::is_same<T, glm::mat4x4>::value) {
        GLFunctionsWrapper::setmat4x4(loc, val);
    } else {
        // if the type has not been implemented this function will fail at compile time
        static_assert(GLFunctionsWrapper::always_false<T>::value);
    }
}

template <typename T>
T Uniform<T>::get() const {
    if constexpr(GLFunctionsWrapper::shader_is_implemented<T>::value) {
        return this->value;
    } else {
        // if the type has not been implemented this function will fail at compile time
        static_assert(always_false<T>::value);
    }
}