#include "gl_helper.hpp"

#include "inventory.hpp"

const uint32_t Inventory::INVENTORY_SPRITE_ID = 0;
const uint32_t Inventory::HOTBAR_SPRITE_ID = 1;


const frect Inventory::INVENTORY_RECT = frect{ 0, 0, 175, 165 };
frect Inventory::inventory_space(uint32_t i, uint32_t j) {
    assert(i < Inventory::INVENTORY_WIDTH);
    assert(j < Inventory::INVENTORY_HEIGHT);

    const uint32_t y_offset = (j != 3 ? 84 : 142);
    const uint32_t y_stride = 18;

    const uint32_t x_offset = 8;
    const uint32_t x_stride = 18;

    return frect{
        static_cast<float>(x_offset + x_stride * i),
        static_cast<float>(y_offset + y_stride * j),
        16,
        16
    };
}
frect Inventory::dest_inventory_space(uint32_t i, uint32_t j, const frect &outer) {
    return apply_equivalent_transformation(
        Inventory::INVENTORY_RECT,
        dest_rect(outer),
        inventory_space(i, j)
    );
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
}

Inventory::~Inventory() {

}

bool Inventory::is_open() {
    return is_shown;
}
void Inventory::toggle() {
    is_shown = !is_shown;
}

Space Inventory::pop_space(uint32_t i, uint32_t j) {
    Space space;
    std::swap(space, inventory[i][j]);
    return space;
}
void Inventory::push_space(uint32_t i, uint32_t j, Space space) {
    std::swap(inventory[i][j], space);
}

void Inventory::draw(const frect &outer, const uint32_t depth) {
    if (is_open()) {
        Renderer::draw(inventory_texture, Inventory::INVENTORY_RECT, dest_rect(outer), depth);
        if (hovered_cell.has_value()) {
            Renderer::draw(
                dest_inventory_space(hovered_cell->x, hovered_cell->y, outer).inset_by(0.1f), 
                color{ 1.f, 1.f, 1.f, 0.2f }, 
                depth + 1
            );
        }
        if (held_item.has_value()) {
            
        }
    }
}
void Inventory::handle_events(const SDL_Event &event, const frect &outer, const uint32_t depth) {
    if (event.type == SDL_MOUSEMOTION) {
        const auto mouse_pos = glm::vec2(static_cast<float>(event.motion.x), static_cast<float>(event.motion.y));

        this->hovered_cell = get_space_from_mouse_pos(mouse_pos, outer);

        this->mouse_pos = mouse_pos;
    }
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        /*
        const auto mouse_pos = glm::vec2(static_cast<float>(event.button.x), static_cast<float>(event.button.y));
        
        std::optional<glm::uvec2> selected_cell = get_space_from_mouse_pos(mouse_pos, outer);
        if (selected_cell.has_value()) {
            
        }
        */
    }
    if (event.type == SDL_MOUSEBUTTONUP) {
        /*
        const auto mouse_pos = glm::vec2(static_cast<float>(event.button.x), static_cast<float>(event.button.y));
        std::optional<glm::uvec2> selected_cell = get_space_from_mouse_pos(mouse_pos, outer);
        */
    }
}
frect Inventory::dest_rect(const frect &outer) const {
    return min_max_scaling(Inventory::INVENTORY_RECT, outer);
}
void Inventory::visit(GUIElementVisitor &visitor) {
    visitor.visit(*this);
}
void Inventory::visit(ConstGUIElementVisitor &visitor) const {
    visitor.visit(*this);
}

std::optional<glm::uvec2> Inventory::get_space_from_mouse_pos(const glm::vec2 &pos, const frect &outer) {
    for (uint32_t i = 0; i < Inventory::INVENTORY_WIDTH; ++i)
        for (uint32_t j = 0; j < Inventory::INVENTORY_HEIGHT; ++j) {
            auto space_rect = dest_inventory_space(i, j, outer);
            if (space_rect.contains(mouse_pos)) {
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