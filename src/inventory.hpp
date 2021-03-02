#pragma once

#include "standard.hpp"
#include "chunk.hpp"

union SDL_Event;

struct Inventory {
    using Space = std::optional<BlockType>;
    static constexpr uint32_t INVENTORY_WIDTH = 9, INVENTORY_HEIGHT = 3, CRAFTING_GRID_LENGTH = 3;
    bool is_shown;

    std::array<Space, INVENTORY_HEIGHT * INVENTORY_WIDTH> inventory;
    std::array<Space, INVENTORY_WIDTH> hotbar;

    std::array<std::array<Space, CRAFTING_GRID_LENGTH>, CRAFTING_GRID_LENGTH> grid;

    Space selected_item;

    bool is_open();
    void toggle();

    Inventory();
    ~Inventory();

    void draw();

    static const uint32_t INVENTORY_SPRITE_ID;
    static const uint32_t HOTBAR_SPRITE_ID;
};