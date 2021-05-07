#include "tool.hpp"

#include "gui.hpp"
#include "inventory.hpp"

frect ToolRects(const Tool &tool) {
    return frect{
        static_cast<float>(static_cast<uint32_t>(tool.type) * 16),
        static_cast<float>(static_cast<uint32_t>(tool.material) * 16),
        16,
        16
    };
}

Tool::Tool(ToolMaterial material, ToolType type)
    : material{ material }, type{ type }, durability(max_durability(material, type)) {

}
uint32_t Tool::max_durability() const {
    return max_durability(this->material, this->type);
}

uint32_t Tool::max_durability(ToolMaterial material, ToolType type) {
    return DURABILITIES.at(material).at(type);
}

template <>
void Renderer::draw(const frect &dest, const uint32_t depth, const Tool &tool) {
    Renderer::draw(dest, depth, Sprite{ *Renderer::tools, ToolRects(tool) });
    if (tool.durability < tool.max_durability()) {
        const frect durability_dest_rect = Inventory::DURABILITY_BAR_RECT.apply_equivalent_transformation(Inventory::CELL_RECT, dest);
        Renderer::draw(durability_dest_rect, depth + 3, color{ 0.5f, .5f, .5f, 1.0f});
        const frect h_durability_dest_rect = {
            durability_dest_rect.x,
            durability_dest_rect.y, 
            (durability_dest_rect.w * static_cast<float>(tool.durability)) / static_cast<float>(tool.max_durability()), 
            durability_dest_rect.h
        };
        Renderer::draw(h_durability_dest_rect, depth + 4, color{ 1.0f, 1.0f, 1.0f, 1.0f });
    }
}