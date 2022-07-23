#include "gl_helper.hpp"
#include "entity.hpp"
#include "world.hpp"
#include "standard.hpp"
#include "player.hpp"

std::optional<Entity> Entity::create(const std::string &name) {
    std::optional<Entity> ret;
    if (name == "player") {
        ret = Entity();
        ret->controller = std::make_unique<PlayerController>();
        ret->type = EntityType::Player;
    }

    return ret;
}

bool rect_intersects(const PhysicalWorld &phys, const std::vector<Entity> &entities, size_t i, BoundingBox box) {
    for (size_t j = 0; j < entities.size(); ++j) {
        if (i != j) {
            BoundingBox bb = {
                entities[j].transform.pos,
                entities[j].transform.size
            };

            if (intersects(bb, box)) {
                return true;
            }
        }
    }

    if (phys.intersects_block(box.pos, box.aabb)) {
        return true;
    }

    return false;
}

void update_entities(PhysicalWorld &phys, std::vector<Entity> &entities, const float dt) {
    for (size_t i = 0; i < entities.size(); ++i) {
        auto &entity = entities[i];
        auto &rigidbody = entities[i].rigidbody;
        auto &movement = entities[i].rigidbody.movement;
        {
            BoundingBox ground_check = {
                {
                    entity.transform.pos.x + 0.95f * entity.transform.size.width,
                    entity.transform.pos.y - entity.transform.size.height - 0.001f,
                    entity.transform.pos.z + 0.95f * entity.transform.size.length,
                },
                {
                    entity.transform.size.width * 0.1f,
                    0.001f,
                    entity.transform.size.length * 0.1f
                }
            };

            entity.rigidbody.on_ground = rect_intersects(phys, entities, i, ground_check);
        }


        vec3 pos = entity.transform.pos;
        vec3 vel = entity.rigidbody.vel;
        const size_t iterations = 10;
        const vec3 acc = vec3{
            0,
            (rigidbody.has_gravity ? -Player::gravity : 0.f),
            0
        };
        const AABB size = entity.transform.size;

        vec3 new_pos = pos + (vel + movement)  * dt;
        vec3 new_vel = vel + acc * dt;

        BoundingBox old_box = { pos, size };
        BoundingBox new_box = { new_pos, size };
        BoundingBox move_box = old_box.union_box(new_box);
        if (!rect_intersects(phys, entities, i, move_box)) {
            pos = new_pos;
            vel = new_vel;
        } else for (size_t h_i = 0; h_i < iterations; ++h_i) {
            BoundingBox old_box = { pos, size };

            new_vel = vel + acc * (dt / iterations);
            new_pos = pos + (vel + movement) * (dt / iterations);

            vec3 components[3] = { vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1) };
            for (size_t a = 0; a < 3; ++a) {
                vec3 delta_pos = components[a] * (vel + movement) * (dt / iterations);
                BoundingBox b = {
                    pos + delta_pos,
                    size
                };
                if (!rect_intersects(phys, entities, i, b)) {
                    pos = pos + delta_pos;
                } else {
                    vel = vel * (1.f - components[a]);
                }
            }
        }

        entity.transform.pos = pos;
        entity.rigidbody.vel = vel;
        entity.rigidbody.movement = { 0, 0 , 0 };
    }
}

PlayerController::PlayerController()
    : up_pressed(false), left_pressed(false), down_pressed(false), right_pressed(false), debug_mode(true) {

}
void PlayerController::init(Entity &entity, PhysicalWorld &world) {
    entity.transform.pos.y = 66.f;
    entity.rigidbody.has_gravity = false;
}
void PlayerController::update(Entity &entity, PhysicalWorld &world, const std::vector<SDL_Event> &events, float dt) {
    world.main_camera.pos(entity.transform.pos + vec3(0.f, 2.f, 0.f));

    for (const auto &event : events) {
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            bool pressed = event.type == SDL_KEYDOWN;
            switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_W:
                up_pressed = pressed;
                break;
            case SDL_SCANCODE_A:
                left_pressed = pressed;
                break;
            case SDL_SCANCODE_S:
                down_pressed = pressed;
                break;
            case SDL_SCANCODE_D:
                right_pressed = pressed;
                break;
            default:;
            }
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
            entity.rigidbody.apply_impulse(vec3(0.f, 4.5f, 0.f));
        }
        if (event.type == SDL_MOUSEMOTION && SDL_GetRelativeMouseMode() == SDL_TRUE) {
            world.main_camera.rotate_right(0.25f * M_PI * event.motion.xrel * dt);
            world.main_camera.rotate_upwards(-0.25f * M_PI * event.motion.yrel * dt);
        }
    }

    glm::vec3 movement = { 0.f, 0.f, 0.f };
    if (up_pressed) {
        movement = movement + world.main_camera.forward();
    }
    if (down_pressed) {
        movement = movement - world.main_camera.forward();
    }
    if (left_pressed) {
        movement = movement - world.main_camera.right();
    }
    if (right_pressed) {
        movement = movement + world.main_camera.right();
    }
    if (!debug_mode) {
        movement = movement * vec3(1.f, 0.f, 1.f);
    }
    float mag = glm::length(movement);
    if (mag >= 0.001f) {
        movement /= mag;
        movement = movement * 1000.f * dt;
    }
    entity.rigidbody.apply_movement(movement);
}