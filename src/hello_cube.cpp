#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL_ttf/SDL_ttf.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>

#include "gl_helper.hpp"

#include "hello_cube.hpp"


std::array<float, 32> cube_vertices = {
    -0.5f,  0.5f, -0.5f, 1.0f,
    -0.5f,  0.5f,  0.5f, 1.0f,
     0.5f,  0.5f, -0.5f, 1.0f,
     0.5f,  0.5f,  0.5f, 1.0f,
     0.5f, -0.5f, -0.5f, 1.0f,
     0.5f, -0.5f,  0.5f, 1.0f,
    -0.5f, -0.5f, -0.5f, 1.0f,
    -0.5f, -0.5f,  0.5f, 1.0f,
};
std::array<uint32_t, 36> cube_indices = {
    // Top face
    0, 1, 2,
    1, 2, 3,
    // Right face
    2, 3, 4,
    3, 4, 5,
    // Bottom face
    4, 5, 6,
    5, 6, 7,
    // Left face
    6, 7, 0,
    7, 0, 1,
    // Back face
    7, 5, 1,
    5, 1, 3,
    // Front face
    6, 4, 0,
    4, 0, 2
};
std::array<uint32_t, 19> cube_edges_lines = {
    0, 1, 3, 2, 0, 6, 4, 5, 7, 6, 0, 1, 7, 1, 3, 5, 3, 2, 4
};

HelloCube::HelloCube() : rotation_around_y_axis{ 0.0f } {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &EBO_edges);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(decltype(cube_vertices)::value_type) * cube_vertices.size(), cube_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(decltype(cube_indices)::value_type) * cube_indices.size(), cube_indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_edges);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(decltype(cube_edges_lines)::value_type) * cube_edges_lines.size(), cube_edges_lines.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(decltype(cube_vertices)::value_type), (void*)0);

    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    shader_program = LoadShaderProgram("shaders/hello_cube.vert", "shaders/hello_cube.frag");
    camera_pos_loc = glGetUniformLocation(shader_program, "camera_pos");
    assert(camera_pos_loc != -1);
    glUniform3f(camera_pos_loc, 0.0f, 0.0f, -4.0f);
    camera_rot_loc = glGetUniformLocation(shader_program, "camera_rot");
    assert(camera_rot_loc != -1);
    glUniform3f(camera_rot_loc, 0.0f, 0.0f, 0.0f);
    object_pos_loc = glGetUniformLocation(shader_program, "object_pos");
    assert(object_pos_loc != -1);
    glUniform3f(object_pos_loc, 0.0f, 0.0f, 0.0f);
    object_rot_loc = glGetUniformLocation(shader_program, "object_rot");
    assert(object_rot_loc != -1);
    glUniform3f(object_rot_loc, 0.0f, 0.0f, 0.0f);
    aspect_ratio_loc = glGetUniformLocation(shader_program, "aspect_ratio");
    assert(aspect_ratio_loc != -1);
    glUniform1f(aspect_ratio_loc, 640.0f / 480.0f);
}
HelloCube::~HelloCube() {
    this->destroy();
}
void HelloCube::destroy() {
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO_edges);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shader_program);
}
HelloCube& HelloCube::operator=(HelloCube &&other) {
    if (this != &other) {
        this->destroy();
        this->EBO = other.EBO;
        this->VBO = other.VBO;
        this->VAO = other.VAO;
        this->shader_program = other.shader_program;
        this->EBO_edges = other.EBO_edges;

        other.EBO = 0;
        other.VBO = 0;
        other.VAO = 0;
        other.shader_program = 0;
        other.EBO_edges = 0;
    }
    return *this;
}

void HelloCube::draw() {
    //auto object_rot_loc = glGetUniformLocation(shader_program, "object_rot");
    //assert(object_rot_loc != -1);
    glUniform3f(object_rot_loc, 0.0f, rotation_around_y_axis, 0.0f);
    glUniform3f(camera_pos_loc, objpos.x, objpos.y, objpos.z);

    auto outColor_loc = glGetUniformLocation(shader_program, "outColor");
    assert(outColor_loc != -1);


    glUniform1f(aspect_ratio_loc, 640.0f / 480.0f);

    glUniform3f(outColor_loc, 1.0f, 1.0f, 1.0f);
    glUseProgram(shader_program);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glDrawElements(GL_TRIANGLES, cube_indices.size(), GL_UNSIGNED_INT, 0);


    glUniform3f(outColor_loc, 1.0f, 0.0f, 0.0f);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_edges);
    glDrawElements(GL_LINE_STRIP, cube_edges_lines.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
void HelloCube::update(const std::vector<SDL_Event> &events) {
    rotation_around_y_axis += 0.01f;
    
    if (keypressed['w']) {
        objpos.z += 0.01f;
    }
    if (keypressed['s']) {
        objpos.z -= 0.01f;
    }
    if (keypressed['a']) {
        objpos.x += 0.01f;
    }
    if (keypressed['d']) {
        objpos.x -= 0.01f;
    }

    for (const auto &event : events) {
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            const bool keystate = event.type == SDL_KEYDOWN;
            switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_W:
                keypressed['w'] = keystate;
                break;
            case SDL_SCANCODE_A:
                keypressed['a'] = keystate;
                break;
            case SDL_SCANCODE_S:
                keypressed['s'] = keystate;
                break;
            case SDL_SCANCODE_D:
                keypressed['d'] = keystate;
                break;
            default: break;
            }
        }
    }
}