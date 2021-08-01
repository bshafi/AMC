#include "rigidbody.hpp"
#include "world.hpp"

Rigidbody::Rigidbody() {
    aabb = AABB(1, 1, 1);
    mass = 1.0f;
    velocity = glm::vec3(0, 0, 0);
    impulse_forces = glm::vec3(0, 0, 0);
    has_gravity = false;
}

void Rigidbody::apply_impulse(const glm::vec3 &force) {
    impulse_forces += force;
}

void Rigidbody::update_pos(glm::vec3 &pos, World &world, const float delta_time) {
    const auto impulse_forces = this->impulse_forces;
    this->impulse_forces = glm::vec3(0, 0, 0);
    
    const glm::vec3 accel_due_to_gravity = (has_gravity ? 9.8f : 0.0f) * glm::vec3(0, -1.0f, 0);
    const glm::vec3 new_vel = accel_due_to_gravity * delta_time + velocity + impulse_forces;
    const glm::vec3 new_pos = velocity * delta_time + pos;

    if (world.intersects_block(new_pos, aabb)) {
        velocity = glm::vec3(0, 0, 0);
    } else {
        pos = new_pos;
        velocity = new_vel;
    }
}
