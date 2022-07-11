#pragma once

#include "physics.hpp"

class PhysicalWorld;

class Entity {
public:
    Transform transform;
    Rigidbody rigidbody;
private:
    friend void update_entities(PhysicalWorld &phys, std::vector<Entity> &, float dt);
};

void update_entities(PhysicalWorld &phys, std::vector<Entity> &, float dt);