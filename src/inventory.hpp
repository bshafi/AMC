#pragma once

#include "standard.hpp"
#include "chunk.hpp"
#include "tool.hpp"
#include "gui.hpp"

union SDL_Event;

struct Space {
    std::variant<Tool, BlockType, std::monostate> item = std::monostate{};
    uint32_t stack_size = 0;
};

class Inventory : public GUIElement {
public:
    static constexpr uint32_t INVENTORY_WIDTH = 9, INVENTORY_HEIGHT = 4, CRAFTING_GRID_LENGTH = 3;

    static const uint32_t INVENTORY_SPRITE_ID;
    static const uint32_t HOTBAR_SPRITE_ID;

    static const frect INVENTORY_RECT;
    static frect inventory_space(uint32_t i, uint32_t j);
    static frect crafting_grid_space(uint32_t i, uint32_t j);
    static const frect CRAFTING_GRID_RESULT_SPACE;
    frect dest_inventory_space(uint32_t i, uint32_t j, const frect &);

    void draw(const frect &outer, const uint32_t depth) override;
    void handle_events(const SDL_Event &event, const frect &outer, const uint32_t depth) override;
    frect dest_rect(const frect &outer) const override;
    void visit(GUIElementVisitor &visitor) override;
    void visit(ConstGUIElementVisitor &visitor) const override;

    bool is_open();
    void toggle();

    Inventory();
    ~Inventory();

    std::optional<glm::uvec2> get_space_from_mouse_pos(const glm::vec2 &, const frect &);
    Space pop_space(uint32_t i, uint32_t j);
    void push_space(uint32_t i, uint32_t j, Space space);
private:

    Texture inventory_texture, hotbar_texture;

    std::array<std::array<Space, INVENTORY_HEIGHT>, INVENTORY_WIDTH> inventory;
    std::optional<glm::uvec2> hovered_cell;
    std::optional<BlockType> held_item;
    glm::vec2 mouse_pos;
    bool is_shown;

    std::array<std::array<Space, CRAFTING_GRID_LENGTH>, CRAFTING_GRID_LENGTH> grid;
};

class InventoryVisitor : public GUIElementVisitor {
public:
    InventoryVisitor(std::function<void(Inventory&)> apply);
    void visit(Inventory &) override;
private:
    std::function<void(Inventory&)> apply;
};