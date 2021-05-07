#pragma once

#include "standard.hpp"

constexpr uint32_t MAX_TOOL_MATERIAL = 2;
enum class ToolMaterial : uint8_t {
    Wood = 0,
    Stone = 1
};

constexpr uint32_t MAX_TOOL_TYPE = 5;
enum class ToolType : uint8_t {
    Pickaxe = 0,
    Shovel = 1,
    Axe = 2,
    Sword = 3,
    Hoe = 4
};

const std::map<ToolMaterial, std::map<ToolType, uint32_t>> DURABILITIES = {
    std::make_pair<ToolMaterial, std::map<ToolType, uint32_t>>(
        ToolMaterial::Wood, 
        {
            { ToolType::Pickaxe, 59 },
            { ToolType::Shovel,  59 },
            { ToolType::Axe,     59 },
            { ToolType::Sword,   59 },
            { ToolType::Hoe,     59 },
        }
    ),
    std::make_pair<ToolMaterial, std::map<ToolType, uint32_t>>(
        ToolMaterial::Stone,
        {
            { ToolType::Pickaxe, 131 },
            { ToolType::Shovel,  131 },
            { ToolType::Axe,     131 },
            { ToolType::Sword,   131 },
            { ToolType::Hoe,     131 },
        }
    ),
};


struct Tool {
    Tool(ToolMaterial, ToolType);
    ToolMaterial material;
    ToolType type;
    uint32_t durability;
    uint32_t max_durability() const;
    static uint32_t max_durability(ToolMaterial, ToolType);
};

frect ToolRects(const Tool &tool);