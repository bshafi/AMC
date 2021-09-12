#pragma once

#include "standard.hpp"
#include "camera.hpp"

struct World;

struct Player {
    glm::vec3 velocity;
    glm::vec3 position;
    AABB aabb;
    Camera camera;
    bool debug_mode;

    static float movement_speed;
    static float jump_speed;
    Player();

    void set_position(const glm::vec3 &);
    void apply_gravity(World&);
    void move_forward(float, World&);
    void move_right(float, World&);
    void look_up(float, World&);
    void look_right(float, World&);
    void jump(World&);
    void toggle_debug_mode(World&);

    static float gravity;
};