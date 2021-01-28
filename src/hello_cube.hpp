#pragma once

#include <glm/glm.hpp>


#include <vector>
#include <map>

union SDL_Event;

const extern std::array<float, 6 * 6 * 5> cube_vertices;

/*
  Hello Cube is a simple rotating textured cube to test the setup of opengl and SDL2
  this will most likely be used to as a small test bed
 */
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