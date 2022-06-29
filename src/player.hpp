#pragma once

#include "standard.hpp"
#include "camera.hpp"

struct World;
struct PhysicalWorld;

struct Player {
    glm::vec3 velocity;
    glm::vec3 position;
    AABB aabb;
    Camera camera;
    bool debug_mode;
    bool on_ground;

    static float movement_speed;
    static float jump_speed;
    Player();

    void set_position(const glm::vec3 &);

    void apply_gravity(PhysicalWorld &, float delta_time_s);
    void move_forward(float, PhysicalWorld&);
    void move_right(float, PhysicalWorld&);
    void look_up(float, PhysicalWorld&);
    void look_right(float, PhysicalWorld&);
    void jump(PhysicalWorld&);
    void toggle_debug_mode();

    static float gravity;
};