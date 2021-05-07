#include "gl_helper.hpp"

#include "inventory.hpp"

const uint32_t Inventory::INVENTORY_SPRITE_ID = 0;
const uint32_t Inventory::HOTBAR_SPRITE_ID = 1;

const frect Inventory::CELL_RECT = { 0, 0, 16, 16 };
const frect Inventory::DURABILITY_BAR_RECT = { 1, 14, 14, 1 };
const frect Inventory::INVENTORY_RECT = frect{ 0, 0, 175, 165 };
frect Inventory::inventory_space(uint32_t i, uint32_t j) {
    assert(i < Inventory::INVENTORY_WIDTH);
    assert(j < Inventory::INVENTORY_HEIGHT);

    const uint32_t y_offset = 84; //(j != 3 ? 84 : 142);
    const uint32_t y_stride = 18;

    const uint32_t x_offset = 8;
    const uint32_t x_stride = 18;

    if (j != 3) {
        return frect{
            static_cast<float>(x_offset + x_stride * i),
            static_cast<float>(y_offset + y_stride * j),
            16,
            16
        };
    } else {
        return frect{
            static_cast<float>(x_offset + x_stride * i),
            static_cast<float>(142),
            16,
            16
        };
    }
}
frect Inventory::dest_inventory_space(uint32_t i, uint32_t j, const frect &outer) {
    return apply_equivalent_transformation(
        Inventory::INVENTORY_RECT,
        h_dest_rect(outer),
        inventory_space(i, j)
    );
}
frect Inventory::dest_crafting_space(uint32_t i, uint32_t j, const frect &outer) {
    assert(i < Inventory::CRAFTING_GRID_LENGTH);
    assert(j < Inventory::CRAFTING_GRID_LENGTH);

    return crafting_grid_space(i, j).apply_equivalent_transformation(Inventory::INVENTORY_RECT, h_dest_rect(outer));
}
frect Inventory::crafting_grid_space(uint32_t i, uint32_t j) {
    assert(i < Inventory::CRAFTING_GRID_LENGTH);
    assert(j < Inventory::CRAFTING_GRID_LENGTH);

    return frect{
        static_cast<float>(80 + 18 * i),
        static_cast<float>(18 + 18 * j),
        16,
        16
    };
}

const frect Inventory::CRAFTING_GRID_RESULT_SPACE = frect{ 154, 36, 16, 16 };

Inventory::Inventory()
    : inventory_texture{ "resources/inventory.png" }, hotbar_texture{ "resources/hotbar.png" } {
    is_shown = false;

    Tool t = Tool(ToolMaterial::Stone, ToolType::Axe);
    t.durability = 23;
    inventory[0][0].item = t;
}

Inventory::~Inventory() {

}

bool Inventory::is_open() {
    return is_shown;
}
void Inventory::toggle() {
    is_shown = !is_shown;
}
template <>
void Renderer::draw(const frect &dest, const uint32_t depth, const Space &space) {
    if (const Tool *tool = std::get_if<Tool>(&space.item)) {
        Renderer::draw(dest, depth, *tool);
    } else if (const BlockType *block_type = std::get_if<BlockType>(&space.item)) {
        Renderer::draw(dest, depth, *block_type);
    }
}

void Inventory::draw(const frect &outer, const uint32_t depth) {
    if (!is_open()) {
        return;
    }
    Renderer::draw(dest_rect(outer), depth, Sprite{ inventory_texture, Inventory::INVENTORY_RECT });
    if (hovered_cell.has_value()) {
        Renderer::draw(
            dest_inventory_space(hovered_cell->x, hovered_cell->y, outer).inset_by(0.1f), 
            depth + 1,
            color{ 1.f, 1.f, 1.f, 0.2f }
        );
    }
    if (selected_crafting_cell.has_value()) {
        Renderer::draw(
            dest_crafting_space(selected_crafting_cell->x, selected_crafting_cell->y, outer).inset_by(0.1f),
            depth + 1,
            color{ 1.f, 1.f, 1.f, 0.2f }
        );
    }
    for (uint32_t i = 0; i < Inventory::INVENTORY_WIDTH; ++i)
        for (uint32_t j = 0; j < Inventory::INVENTORY_HEIGHT; ++j) {
            const frect space_rect = dest_inventory_space(i, j, outer);
            Renderer::draw(space_rect, depth + 2, inventory[i][j]);
        }
    for (uint32_t i = 0; i < CRAFTING_GRID_LENGTH; ++i)
        for (uint32_t j = 0; j < CRAFTING_GRID_LENGTH; ++j) {
            const frect space_rect = dest_crafting_space(i, j, outer);
            Renderer::draw(space_rect, depth + 2, grid[i][j]);
        }
    
    {
        const frect temp_rect = dest_inventory_space(0, 0, outer);
        const frect space_rect = {
            mouse_pos.x - temp_rect.w / 2.0f,
            mouse_pos.y - temp_rect.h / 2.0f,
            temp_rect.w,
            temp_rect.h
        };
        Renderer::draw(space_rect, depth + 10, held_item);
    }
}
void Inventory::handle_events(const SDL_Event &event, const frect &outer, const uint32_t depth) {
    if (event.type == SDL_MOUSEMOTION) {
        const auto mouse_pos = glm::vec2(static_cast<float>(event.motion.x), static_cast<float>(event.motion.y));

        this->hovered_cell = get_space_from_mouse_pos(mouse_pos, outer);
        
        this->mouse_pos = mouse_pos;

        this->selected_crafting_cell = get_crafting_space_from_mouse_pos(mouse_pos, outer);
    }
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (this->hovered_cell.has_value()) {
            assert(hovered_cell->x < INVENTORY_WIDTH);
            assert(hovered_cell->y < INVENTORY_HEIGHT);

            std::swap(inventory[this->hovered_cell->x][this->hovered_cell->y], this->held_item);
        }
        if (this->selected_crafting_cell.has_value()) {
            assert(selected_crafting_cell->x < CRAFTING_GRID_LENGTH);
            assert(selected_crafting_cell->y < CRAFTING_GRID_LENGTH);

            std::swap(grid[this->selected_crafting_cell->x][this->selected_crafting_cell->y], this->held_item);
        }
    }
}
frect Inventory::h_dest_rect(const frect &outer) {
    return min_max_scaling(Inventory::INVENTORY_RECT, outer).inset_by(0.1f);
}
frect Inventory::dest_rect(const frect &outer) const {
    return h_dest_rect(outer);
}
void Inventory::visit(GUIElementVisitor &visitor) {
    visitor.visit(*this);
}
void Inventory::visit(ConstGUIElementVisitor &visitor) const {
    visitor.visit(*this);
}

std::optional<glm::uvec2> Inventory::get_space_from_mouse_pos(const glm::vec2 &pos, const frect &outer) const {
    for (uint32_t i = 0; i < Inventory::INVENTORY_WIDTH; ++i)
        for (uint32_t j = 0; j < Inventory::INVENTORY_HEIGHT; ++j) {
            auto space_rect = dest_inventory_space(i, j, outer);
            if (space_rect.contains(mouse_pos)) {
                return glm::uvec2(i, j);
            }
        }
    return std::nullopt;
}

std::optional<glm::uvec2> Inventory::get_crafting_space_from_mouse_pos(const glm::vec2 &pos, const frect &outer) const {
    for (uint32_t i = 0; i < Inventory::CRAFTING_GRID_LENGTH; ++i)
        for (uint32_t j = 0; j < Inventory::CRAFTING_GRID_LENGTH; ++j) {
            auto space_rect = dest_crafting_space(i, j, outer);
            if (space_rect.contains(pos)) {
                return glm::uvec2(i, j);
            }
        }
    return std::nullopt;
}

InventoryVisitor::InventoryVisitor(std::function<void(Inventory&)> apply)
    : apply{apply} {

}
void InventoryVisitor::visit(Inventory &inventory) {
    apply(inventory);
}