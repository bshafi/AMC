#include <array>

#include "gl_helper.hpp"
#include "hello_cube.hpp"

const std::array<float, 6 * 6 * 5> cube_vertices = {
    -0.5f,  0.5f,  0.5f, 0.f / 6, 0.0f,
    -0.5f,  0.5f, -0.5f, 0.f / 6, 1.0f,
     0.5f,  0.5f, -0.5f, 1.f / 6, 1.0f,

    -0.5f,  0.5f,  0.5f, 0.f / 6, 0.0f,
     0.5f,  0.5f, -0.5f, 1.f / 6, 1.0f,
     0.5f,  0.5f,  0.5f, 1.f / 6, 0.0f,


    -0.5f, -0.5f,  0.5f, 1.f / 6, 0.0f,
    -0.5f, -0.5f, -0.5f, 1.f / 6, 1.0f,
     0.5f, -0.5f, -0.5f, 2.f / 6, 1.0f,

    -0.5f, -0.5f,  0.5f, 1.f / 6, 0.0f,
     0.5f, -0.5f, -0.5f, 2.f / 6, 1.0f,
     0.5f, -0.5f,  0.5f, 2.f / 6, 0.0f,


    -0.5f, -0.5f,  0.5f, 2.f / 6, 0.0f,
    -0.5f,  0.5f,  0.5f, 2.f / 6, 1.0f,
     0.5f,  0.5f,  0.5f, 3.f / 6, 1.0f,

    -0.5f, -0.5f,  0.5f, 2.f / 6, 0.0f,
     0.5f,  0.5f,  0.5f, 3.f / 6, 1.0f,
     0.5f, -0.5f,  0.5f, 3.f / 6, 0.0f,


    -0.5f, -0.5f, -0.5f, 3.f / 6, 0.0f,
    -0.5f,  0.5f, -0.5f, 3.f / 6, 1.0f,
     0.5f,  0.5f, -0.5f, 4.f / 6, 1.0f,
    
    -0.5f, -0.5f, -0.5f, 3.f / 6, 0.0f,
     0.5f,  0.5f, -0.5f, 4.f / 6, 1.0f,
     0.5f, -0.5f, -0.5f, 4.f / 6, 0.0f,

    
     0.5f, -0.5f,  0.5f, 4.f / 6, 0.0f,
     0.5f,  0.5f,  0.5f, 4.f / 6, 1.0f,
     0.5f,  0.5f, -0.5f, 5.f / 6, 1.0f,

     0.5f, -0.5f,  0.5f, 4.f / 6, 0.0f,
     0.5f,  0.5f, -0.5f, 5.f / 6, 1.0f,
     0.5f, -0.5f, -0.5f, 5.f / 6, 0.0f,


    -0.5f, -0.5f, -0.5f, 5.f / 6, 0.0f,
    -0.5f,  0.5f, -0.5f, 5.f / 6, 1.0f,
    -0.5f,  0.5f,  0.5f, 6.f / 6, 1.0f,

    -0.5f, -0.5f, -0.5f, 5.f / 6, 0.0f,
    -0.5f,  0.5f,  0.5f, 6.f / 6, 1.0f,
    -0.5f, -0.5f,  0.5f, 6.f / 6, 0.0f
};

// refrence image ./drawing.svg
std::array<float, 16 * 5> cube_verts = {
    -1,  1, -1, 0.f / 6, 0.0f, // 1
     1,  1, -1, 0.f / 6, 1.0f, // 0
    -1,  1,  1, 1.f / 6, 0.0f, // 2
     1,  1,  1, 1.f / 6, 1.0f, // 3
    -1, -1,  1, 2.f / 6, 0.0f, // 4
     1, -1,  1, 2.f / 6, 1.0f, // 5
    -1, -1, -1, 3.f / 6, 0.0f, // 6
     1, -1, -1, 3.f / 6, 1.0f, // 7

     1, -1,  1, 3.f / 6, 1.0f, // 8
     1,  1,  1, 3.f / 6, 0.0f, // 9
     1, -1, -1, 4.f / 6, 1.0f, // 10
     1,  1, -1, 4.f / 6, 0.0f, // 11
    -1, -1, -1, 5.f / 6, 1.0f, // 12
    -1,  1, -1, 5.f / 6, 0.0f, // 13
    -1, -1,  1, 6.f / 6, 1.0f, // 14
    -1,  1,  1, 6.f / 6, 0.0f  // 15
};

std::array<uint32_t, 36> cube_indices = {
    // Top face
    0, 1, 2,
    1, 2, 3,
    // Front face
    2, 3, 4,
    3, 4, 5,
    // Bottom face
    4, 5, 6,
    5, 6, 7,
    // Duplicate cube_indices are for other textures
    // Right face
    8, 9, 10,
    9, 10, 11,
    // Posterior face (opposite of front)
    10, 11, 12,
    11, 12, 13,
    // Left face
    12, 13, 14,
    13, 14, 15
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(decltype(cube_vertices)::value_type), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(decltype(cube_vertices)::value_type), (void*)(3 * sizeof(decltype(cube_vertices)::value_type)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(decltype(cube_indices)::value_type) * cube_indices.size(), cube_indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_edges);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(decltype(cube_edges_lines)::value_type) * cube_edges_lines.size(), cube_edges_lines.data(), GL_STATIC_DRAW);


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

    tex_loc = glGetUniformLocation(shader_program, "tex");
    glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
    glBindTexture(GL_TEXTURE_2D, block_texture);
    

    block_texture = LoadImage("resources/hello_cube_orientation.png", nullptr, nullptr);
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
    glDeleteTextures(1, &block_texture);
}
HelloCube& HelloCube::operator=(HelloCube &&other) {
    if (this != &other) {
        this->destroy();
        this->EBO = other.EBO;
        this->VBO = other.VBO;
        this->VAO = other.VAO;
        this->shader_program = other.shader_program;
        this->EBO_edges = other.EBO_edges;
        this->block_texture = other.block_texture;

        other.EBO = 0;
        other.VBO = 0;
        other.VAO = 0;
        other.shader_program = 0;
        other.EBO_edges = 0;
        other.block_texture = 0;
    }
    return *this;
}

void HelloCube::draw() {
    //auto object_rot_loc = glGetUniformLocation(shader_program, "object_rot");
    //assert(object_rot_loc != -1);
    glUniform3f(object_rot_loc, 0.0f, rotation_around_y_axis, 0.0f);
    glUniform3f(camera_pos_loc, objpos.x, objpos.y, objpos.z);
    glUniform3f(camera_rot_loc, camerarot.x, camerarot.y, camerarot.z);

    glUniform1f(aspect_ratio_loc, 640.0f / 480.0f);

    glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
    glBindTexture(GL_TEXTURE_2D, block_texture);

    glUniform1i(tex_loc, 0);

    glUseProgram(shader_program);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //glDrawElements(GL_TRIANGLES, cube_indices.size(), GL_UNSIGNED_INT, 0);
    glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size() / 5);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_edges);
    glDrawElements(GL_LINE_STRIP, cube_edges_lines.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void HelloCube::update(const std::vector<SDL_Event> &events) {
    //rotation_around_y_axis += 0.01f;
    glm::vec3 forward = glm::vec3(1.f, 0.f, 0.f);
    if (keypressed['w']) {
        //objpos.z += 0.01f;
        objpos += forward * 0.01f;
    }
    if (keypressed['s']) {
        //objpos.z -= 0.01f;
        objpos -= forward * 0.01f;
    }
    if (keypressed['a']) {
        objpos.x += 0.01f;
    }
    if (keypressed['d']) {
        objpos.x -= 0.01f;
    }
    if (keypressed['i']) {
        objpos.y += 0.01f;
    }
    if (keypressed['k']) {
        objpos.y -= 0.01f;
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
            case SDL_SCANCODE_I:
                keypressed['i'] = keystate;
                break;
            case SDL_SCANCODE_K:
                keypressed['k'] = keystate;
                break;
            case SDL_SCANCODE_ESCAPE:
                SDL_SetRelativeMouseMode(SDL_FALSE);
                break;
            default: break;
            }
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                SDL_SetRelativeMouseMode(SDL_TRUE);
            }
        } else if (event.type == SDL_MOUSEMOTION && SDL_GetRelativeMouseMode() == SDL_TRUE) {
            camerarot.y += event.motion.xrel / 100.f;
            camerarot.x += event.motion.yrel / 100.f;
        }
    }
}