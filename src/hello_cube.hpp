#pragma once

#include <glm/glm.hpp>

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