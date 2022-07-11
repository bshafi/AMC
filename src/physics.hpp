#pragma once

#include "standard.hpp"

struct Transform {
    Transform(vec3 pos = { 0, 0, 0 }, AABB size = { 1, 1, 1 })
        : pos(pos), size(size) {
        
    }

    vec3 pos;
    AABB size;
};

struct Rigidbody {
    vec3 movement = { 0, 0, 0 };
    vec3 vel = { 0, 0, 0 };
    bool has_gravity = true;
    bool on_ground = false;

    void apply_impulse(vec3 impulse);
    void apply_movement(vec3 movement); 
};