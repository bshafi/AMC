#pragma once

#include "standard.hpp"

struct PhysicalWorld;

class Rigidbody {
public:
    Rigidbody();

    void apply_impulse(const glm::vec3 &force);
    void update_pos(glm::vec3 &pos, PhysicalWorld &world, const float delta_time);

    AABB aabb;
    float mass;
    glm::vec3 velocity;
    glm::vec3 impulse_forces;
    bool has_gravity;
};

