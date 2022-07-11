#include "physics.hpp"

void Rigidbody::apply_impulse(vec3 impulse) {
    this->vel = this->vel + impulse;
}
void Rigidbody::apply_movement(vec3 movement) {
    this->movement = this->movement + movement;
}