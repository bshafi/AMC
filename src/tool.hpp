#pragma once

#include "standard.hpp"

enum class ToolMaterial : uint8_t {
    Wood = 0,
    Stone = 1
};

enum class ToolType : uint8_t {
    Pickaxe = 0,
    Shovel = 1,
    Axe = 2,
    Sword = 3,
    Hoe = 4
};

struct Tool {
    Tool(ToolMaterial, ToolType);
    ToolMaterial material;
    ToolType type;
    uint32_t durability;
    static uint32_t max_durability(ToolMaterial, ToolType);
};