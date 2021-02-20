
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.hpp"


const glm::vec3 Camera::UP = glm::vec3(0, 1, 0);

Camera::Camera(glm::vec3 camera_pos) : camera_pos{camera_pos}, _yaw{-M_PI_2}, _pitch{0.0f} {

}

glm::vec3 Camera::pos() const {
    return camera_pos;
}
void Camera::pos(glm::vec3 pos) {
    this->camera_pos = pos;
}

glm::mat4x4 Camera::view_matrix() const {
    return glm::lookAt(camera_pos, camera_pos + forward(), UP);
}

glm::vec3 Camera::forward() const {
    return glm::normalize(glm::vec3(
        cos(_yaw) * cos(_pitch),
        sin(_pitch),
        sin(_yaw) * cos(_pitch)
    ));
}
glm::vec3 Camera::right() const {
    return glm::cross(forward(), UP);
}
glm::vec3 Camera::up() const {
    return glm::cross(forward(), right());
}

void Camera::rotate_upwards(float f) {
    _pitch += f;
}
void Camera::rotate_right(float f) {
    _yaw += f;
}

void Camera::pitch(float p) {
    _pitch = p;
}
float Camera::pitch() const {
    return _pitch;
}

void Camera::yaw(float y) {
    _yaw = y;
}
float Camera::yaw() const {
    return _yaw;
}