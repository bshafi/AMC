#include "gl_helper.hpp"

#include "inventory.hpp"

const uint32_t Inventory::INVENTORY_SPRITE_ID = 0;
const uint32_t Inventory::HOTBAR_SPRITE_ID = 1;


Inventory::Inventory() {
    is_shown = false;
}

Inventory::~Inventory() {

}

bool Inventory::is_open() {
    return is_shown;
}
void Inventory::toggle() {
    is_shown = !is_shown;
}

void Inventory::draw() {
}