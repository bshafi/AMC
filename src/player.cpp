#include "world.hpp"
#include "player.hpp"

Player::Player() {
    debug_mode = true;
    aabb = { 0.9, 2, 0.9 };
    velocity = glm::vec3(0, 0, 0);
}


void Player::set_position(const glm::vec3 &pos) {
    this->position = pos;
    camera.pos(pos + glm::vec3(0, 1, 0));
}

// FIXME: The y velocity is nonzero when the player is not falling
void Player::apply_gravity(World &w) {
    if (!debug_mode) {
        auto position_copy = this->position;
        auto velocity_copy = this->velocity;

        this->velocity = velocity_copy + glm::vec3(0, -gravity, 0);
        set_position(w.try_move_to(position_copy, velocity_copy, this->aabb));
        
        // uses this->velocity instead of velocity_copy because the 
        if (w.intersects_block(position_copy + this->velocity, this->aabb)) {
            velocity *= glm::vec3(1, 0, 1); // zero the vertical velocity
        }
    }
}
void Player::move_forward(float f, World &w) {
    if (debug_mode) {
        set_position(w.try_move_to(this->position, this->camera.forward() * speed * f, this->aabb));
    } else {
        glm::vec2 forward = { cos(camera.yaw()), sin(camera.yaw()) };
        set_position(w.try_move_to(this->position, glm::vec3(forward.x, 0, forward.y) * speed * f, this->aabb));
    }
}
void Player::move_right(float f, World &w) {
    if (debug_mode) {
        set_position(w.try_move_to(this->position, this->camera.right() * speed * f, this->aabb));
    } else {
        glm::vec2 right = { -sin(camera.yaw()), cos(camera.yaw()) };
        set_position(w.try_move_to(this->position, glm::vec3(right.x, 0, right.y) * speed * f, this->aabb));
    }
}
void Player::look_up(float f, World &w) {
    this->camera.rotate_upwards(f);
}
void Player::look_right(float f, World &w) {
    this->camera.rotate_right(f);
}
void Player::jump(World &w) {
    if (fabs(this->velocity.y) < 0.01) {
        this->velocity.y = .8f;
    } else {
        std::cout << "y velocity was " << velocity.y << std::endl;
    }
}
void Player::toggle_debug_mode(World &w) {
    debug_mode = !debug_mode;
    std::cout << "debug_mode: " << (debug_mode ? "enabled" : "disabled") << std::endl;
}