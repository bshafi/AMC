#pragma once

#include <optional>

#include "physics.hpp"
#include "model.hpp"

class PhysicalWorld;
class Entity;

class EntityController {
public:
    virtual void init(Entity &, PhysicalWorld &) = 0;
    virtual void update(Entity &, PhysicalWorld &, const std::vector<SDL_Event> &events, float dt) = 0;
    virtual ~EntityController() {}
private:
};

class Entity {
public:
    Transform transform;
    Rigidbody rigidbody;

    std::unique_ptr<EntityController> controller;
    std::optional<FullMCModel> model;
private:
    friend void update_entities(PhysicalWorld &phys, std::vector<Entity> &, float dt);
};

void update_entities(PhysicalWorld &phys, std::vector<Entity> &, float dt);

class PlayerController : public EntityController {
public:
    PlayerController();
    void init(Entity &, PhysicalWorld &) override;
    void update(Entity &, PhysicalWorld &, const std::vector<SDL_Event> &event, float dt) override;
private:
    bool up_pressed, left_pressed, down_pressed, right_pressed;
    bool debug_mode;
};