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


    void apply_gravity_phys(PhysicalWorld &);
    void move_forward_phys(float, PhysicalWorld&);
    void move_right_phys(float, PhysicalWorld&);
    void look_up_phys(float, PhysicalWorld&);
    void look_right_phys(float, PhysicalWorld&);
    void jump_phys(PhysicalWorld&);
    void toggle_debug_mode();

    static float gravity;
};