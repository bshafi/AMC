#pragma once

#include "gui.hpp"

class Container : public GUIElement {
public:
    Container(const frect &container_rect, const pair_vector<GUIElement*, frect> &elements);
    ~Container();

    void draw(Renderer &, const frect &outer, const uint32_t depth) override;
    void handle_events(const SDL_Event &event, const frect &outer, const uint32_t depth) override;
    frect dest_rect(const frect &outer) const override;
    void visit(GUIElementVisitor &visitor) override;
    void visit(ConstGUIElementVisitor &) const override;
private:
    pair_vector<GUIElement*, frect> elements;
    frect container_rect;
};