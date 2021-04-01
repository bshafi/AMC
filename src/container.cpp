#include "container.hpp"

Container::Container(const frect &container_rect, const pair_vector<GUIElement*, frect> &elements)
    : container_rect{ container_rect }, elements{ elements } {

}
Container::~Container() {
    for (auto [element, rect] : elements) {
        assert(element);
        REMOVE_POINTER(element);
        delete element;
    }
}

void Container::draw(Renderer &renderer, const frect &outer, const uint32_t depth) {
    
}
void Container::handle_events(const SDL_Event &event, const frect &outer, const uint32_t depth) {

}
frect Container::dest_rect(const frect &outer) const {

}
void Container::visit(GUIElementVisitor &visitor) {

}
void Container::visit(ConstGUIElementVisitor &) const {


}