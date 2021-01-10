#pragma once

#include <glm/glm.hpp>

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
    void update();
private:
    void destroy();
    unsigned int VBO, EBO, VAO;
    unsigned int EBO_edges;

    unsigned int shader_program;
    float rotation_around_y_axis;
};