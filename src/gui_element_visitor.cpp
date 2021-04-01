#include "gui_element_visitor.hpp"

#include "gui.hpp"
#include "inventory.hpp"


void GUIElementVisitor::visit(GUIElement *element, const frect &outer, const uint32_t depth) {
    element->visit(*this);
}
void GUIElementVisitor::visit(Sprite &) {}
void GUIElementVisitor::visit(ZStack &) {}
void GUIElementVisitor::visit(Button &) {}
void GUIElementVisitor::visit(HBisection &) {}
void GUIElementVisitor::visit(VBisection &) {}
void GUIElementVisitor::visit(Container &) {}
void GUIElementVisitor::visit(Inventory &) {}


void ConstGUIElementVisitor::visit(GUIElement * const element, const frect &outer, const uint32_t depth) {
    element->visit(*this);
}
void ConstGUIElementVisitor::visit(const Sprite &) {}
void ConstGUIElementVisitor::visit(const ZStack &) {}
void ConstGUIElementVisitor::visit(const Button &) {}
void ConstGUIElementVisitor::visit(const HBisection &) {}
void ConstGUIElementVisitor::visit(const VBisection &) {}
void ConstGUIElementVisitor::visit(const Container &) {}
void ConstGUIElementVisitor::visit(const Inventory &) {}