#pragma once

#include "standard.hpp"

class Sprite;
class ZStack;
class Button;
class HBisection;
class VBisection;
class Container;
class GUIElement;
class Inventory;

class GUIElementVisitor {
public:
    void visit(GUIElement *element, const frect &outer, const uint32_t depth);
    virtual void visit(Sprite &);
    virtual void visit(ZStack &);
    virtual void visit(Button &);
    virtual void visit(HBisection &);
    virtual void visit(VBisection &);
    virtual void visit(Container &);
    virtual void visit(Inventory &);
private:
};
class ConstGUIElementVisitor {
public:
    void visit(GUIElement * const element, const frect &outer, const uint32_t depth);
    virtual void visit(const Sprite &);
    virtual void visit(const ZStack &);
    virtual void visit(const Button &);
    virtual void visit(const HBisection &);
    virtual void visit(const VBisection &);
    virtual void visit(const Container &);
    virtual void visit(const Inventory &);
};