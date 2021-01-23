#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    Camera(glm::vec3 camera_pos = glm::vec3(0, 0, 0));

    glm::vec3 pos() const;
    void pos(glm::vec3 pos);
    glm::mat4x4 view_matrix() const;

    glm::vec3 forward() const;
    glm::vec3 right() const;
    glm::vec3 up() const;

    void rotate_upwards(float f);
    void rotate_right(float f);

    void pitch(float p);
    float pitch() const;

    void yaw(float y);
    float yaw() const;
private:
    glm::vec3 camera_pos;
    // Note: pitch is up and down while yaw is left and right
    float _yaw, _pitch;

    static glm::vec3 UP;
};