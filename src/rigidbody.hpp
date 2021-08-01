#pragma once

#include "standard.hpp"

struct World;

class Rigidbody {
public:
    Rigidbody();

    void apply_impulse(const glm::vec3 &force);
    void update_pos(glm::vec3 &pos, World &world, const float delta_time);

    AABB aabb;
    float mass;
    glm::vec3 velocity;
    glm::vec3 impulse_forces;
    bool has_gravity;
};

