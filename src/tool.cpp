#include "tool.hpp"


Tool::Tool(ToolMaterial material, ToolType type)
    : material{ material }, type{ type }, durability(max_durability(material, type)) {

}

uint32_t Tool::max_durability(ToolMaterial material, ToolType type) {
    switch (type) {
    case ToolType::Pickaxe:
        switch (material) {
        case ToolMaterial::Wood:
            return 59;
            break;
        case ToolMaterial::Stone:
            return 131;
            break;
        }
        break;
    case ToolType::Shovel:
        switch (material) {
        case ToolMaterial::Wood:
            return 59;
            break;
        case ToolMaterial::Stone:
            return 131;
            break;
        }
        break;
    case ToolType::Axe:
        switch (material) {
        case ToolMaterial::Wood:
            return 59;
            break;
        case ToolMaterial::Stone:
            return 131;
            break;
        }
        break;
    case ToolType::Sword:
        switch (material) {
        case ToolMaterial::Wood:
            return 59;
            break;
        case ToolMaterial::Stone:
            return 131;
            break;
        }
        break;
    case ToolType::Hoe:
        switch (material) {
        case ToolMaterial::Wood:
            return 59;
            break;
        case ToolMaterial::Stone:
            return 131;
            break;
        }
        break;
    }
}