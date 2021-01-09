#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <map>

union SDL_Event;

class HelloCube {
public:
    HelloCube();
    ~HelloCube();

    HelloCube(const HelloCube&) = delete;
    HelloCube& operator=(HelloCube &&);

    void draw();
    void update(const std::vector<SDL_Event>&);
private:
    void destroy();
    unsigned int VBO, EBO, VAO;
    unsigned int EBO_edges;

    unsigned int shader_program;
    float rotation_around_y_axis;
    std::map<char, bool> keypressed;

    unsigned int block_texture;

    int camera_pos_loc, camera_rot_loc, object_pos_loc, object_rot_loc, aspect_ratio_loc, tex_loc;
    glm::vec3 objpos, camerarot;
};