#include "world.hpp"
#include "player.hpp"

float Player::gravity = 9.8f;
float Player::movement_speed = 0.7f;
float Player::jump_speed = 0.5f;

Player::Player() {
    debug_mode = true;
    aabb = { 0.9, 2, 0.9 };
    velocity = glm::vec3(0, 0, 0);
    on_ground = false;
}


void Player::set_position(const glm::vec3 &pos) {
    this->position = pos;
    camera.pos(pos + glm::vec3(0, 1, 0));
}

void Player::apply_gravity(PhysicalWorld &w, float delta_time_s) {
    if (!debug_mode) {
        auto position_copy = this->position;
        auto velocity_copy = this->velocity;

        this->velocity = velocity_copy + glm::vec3(0, -gravity * delta_time_s, 0);
        set_position(w.try_move_to(position_copy, velocity_copy, this->aabb));
    }

    AABB feet_aabb = {
        0.9f * this->aabb.width,
        0.1f * this->aabb.height,
        0.9f * this->aabb.length
    };
    glm::vec3 feet_pos = {
        this->position.x + (0.1f * this->aabb.width / 2),
        this->position.y + (-1.f * this->aabb.height),
        this->position.z + (0.1f * this->aabb.length / 2)
    };
    on_ground = w.intersects_block(feet_pos, feet_aabb);
}
void Player::move_forward(float f, PhysicalWorld &w) {
    if (debug_mode) {
        set_position(w.try_move_to(this->position, this->camera.forward() * movement_speed * f, this->aabb));
    } else {
        glm::vec2 forward = { cos(camera.yaw()), sin(camera.yaw()) };
        set_position(w.try_move_to(this->position, glm::vec3(forward.x, 0, forward.y) * movement_speed * f, this->aabb));
    }
}
void Player::move_right(float f, PhysicalWorld &w) {
    if (debug_mode) {
        set_position(w.try_move_to(this->position, this->camera.right() * movement_speed * f, this->aabb));
    } else {
        glm::vec2 right = { -sin(camera.yaw()), cos(camera.yaw()) };
        set_position(w.try_move_to(this->position, glm::vec3(right.x, 0, right.y) * movement_speed * f, this->aabb));
    }
}
void Player::look_up(float f, PhysicalWorld &w) {
    this->camera.rotate_upwards(f);
}
void Player::look_right(float f, PhysicalWorld &w) {
    this->camera.rotate_right(f);
}
void Player::jump(PhysicalWorld &w) {
    if (on_ground) {
        this->velocity.y = jump_speed;
    }
}

void Player::toggle_debug_mode() {
    debug_mode = !debug_mode;
    std::cout << "debug_mode: " << (debug_mode ? "enabled" : "disabled") << std::endl;
}