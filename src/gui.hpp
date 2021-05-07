#pragma once

#include "standard.hpp"
#include "shader.hpp"
#include "chunk.hpp"
#include "gui_element_visitor.hpp"


union SDL_Event;

#ifdef MANUAL_LEAK_CHECK
    extern std::map<uintptr_t, std::pair<std::string, uint32_t>> pointers;

    #define ADD_POINTER(p)  \
        assert(pointers.find(reinterpret_cast<uintptr_t>(p)) == pointers.end()); \
        pointers[reinterpret_cast<uintptr_t>(p)] = std::pair<std::string, uint32_t>{ std::string(__FILE__), __LINE__ }
    #define REMOVE_POINTER(p) \
        assert(pointers.find(reinterpret_cast<uintptr_t>(p)) != pointers.end()); \ 
        pointers.erase(pointers.find(reinterpret_cast<uintptr_t>(p)))
#else
    #define ADD_POINTER(p) do { } while (false)
    #define REMOVE_POINTER(p) do { } while (false)
#endif

struct Sprite {
    const Texture &texture;
    const frect &source;
};

struct Tool;

class Renderer {
public:
    //static void draw(const Texture&, const frect &source, const frect &dest, const uint32_t &depth = 0);
    //static void draw(const frect &, const color &color, const uint32_t &depth = 0);
    //static void draw(const frect &, const BlockType &block_type, const uint32_t &depth = 0);

    template <typename T>
    static void draw(const frect& dest, const uint32_t depth, const T&);

    static void Initialize();
    static void Terminate();
private:
    static uint32_t corners_VBO;
    static uint32_t sprite_VAO;
    static std::unique_ptr<Shader> shader;
    static std::unique_ptr<Texture> tools;
    static std::unique_ptr<Texture> blocks;
};

class GUIElementVisitor;
class ConstGUIElementVisitor;

class GUIElement {
public:
    virtual void draw(const frect &outer, const uint32_t depth) {}
    virtual void handle_events(const SDL_Event &event, const frect &outer, const uint32_t depth) {}
    virtual frect dest_rect(const frect &outer) const = 0;
    virtual void visit(GUIElementVisitor &visitor) = 0;
    virtual void visit(ConstGUIElementVisitor &) const = 0;
    virtual ~GUIElement() {}
private:
};

class GUIImage : public GUIElement {
public:
    GUIImage(Texture &texture, const frect &source, bool should_stretch = false);
    GUIImage(Texture &texture, bool should_stretch = false);
    ~GUIImage() {

    }


    void draw(const frect &outer, const uint32_t depth) override;
    frect dest_rect(const frect &outer) const override;
    void visit(GUIElementVisitor &visitor) override {};
    void visit(ConstGUIElementVisitor &visitor) const override {};
private:
    Texture &texture;
    frect source;
    bool should_stretch;
};

class ZStack : public GUIElement {
public:
    ZStack(const std::vector<GUIElement*> &layers);
    ~ZStack();

    void draw(const frect &outer, const uint32_t depth) override;
    void handle_events(const SDL_Event &event, const frect &outer, const uint32_t depth) override;
    frect dest_rect(const frect &outer) const override;
    void visit(GUIElementVisitor &visitor) override;
    void visit(ConstGUIElementVisitor &) const override;

    ZStack& operator=(ZStack);
    ZStack(ZStack &&);
    friend void swap(ZStack &, ZStack &);
private:
    std::vector<GUIElement*> layers;
};

class Button : public GUIElement {
public:
    Button(Texture &, const frect &normal, const frect &on_hover, uint32_t id = 0);

    frect source_rect() const;

    void draw(const frect &outer, const uint32_t depth) override;
    void handle_events(const SDL_Event &event, const frect &outer, const uint32_t depth) override;
    frect dest_rect(const frect &outer) const override;
    void visit(GUIElementVisitor &) override;
    void visit(ConstGUIElementVisitor &) const override;

    bool clicked() const;
    uint32_t id() const;
private:
    Texture &texture;
    frect normal, on_hover;
    uint32_t h_id;
    bool is_clicked, mouse_is_hovering;
};

class HBisection : public GUIElement {
public:
    HBisection(GUIElement *, GUIElement *, float top_section = 0.5f);
    ~HBisection();

    void draw(const frect &outer, const uint32_t depth) override;
    void handle_events(const SDL_Event &event, const frect &outer, const uint32_t depth) override;
    frect dest_rect(const frect &outer) const override;
    void visit(GUIElementVisitor &) override;
    void visit(ConstGUIElementVisitor &) const override;

    HBisection& operator=(HBisection);
    HBisection(HBisection &&);
    friend void swap(HBisection &, HBisection &);
    
    frect top_rect(const frect &) const;
    frect bottom_rect(const frect &) const;
private:
    GUIElement *top, *bottom;
    float top_section;
};

class VBisection : public GUIElement {
public:
    VBisection(GUIElement *, GUIElement *, float left_section = 0.5f);
    ~VBisection();

    void draw(const frect &outer, const uint32_t depth) override;
    void handle_events(const SDL_Event &event, const frect &outer, const uint32_t depth) override;
    frect dest_rect(const frect &outer) const override;
    void visit(GUIElementVisitor &) override;
    void visit(ConstGUIElementVisitor &) const override;

    VBisection& operator=(VBisection);
    VBisection(VBisection &&);
    friend void swap(VBisection &, VBisection &);

    frect left_rect(const frect &) const;
    frect right_rect(const frect &) const;
private:
    GUIElement *left, *right;
    float left_section;
};

class ButtonClickedVisitor : public ConstGUIElementVisitor {
public:
    ButtonClickedVisitor(uint32_t id);
    void visit(const Button&) override;

    bool is_clicked() const;
private:
    uint32_t id;
    bool h_is_clicked;
};

struct GUI {
    std::vector<Texture*> textures;
    GUIElement *root;
    glm::uvec2 window_bounds;
    glm::ivec2 fake_window_bounds;

    GUI();
    ~GUI();

    //using Visitor = std::function<void(GUIElement *, Texture&, const frect&, const frect&, const uint32_t&)>;

    // Takes ownership both of these should only be called once
    void set_textures(std::vector<Texture*> &);
    void set_root(GUIElement **root);

    //void apply_to_leaves(Visitor, HGUIElement *element, const frect &outer, const uint32_t &depth);
    void draw(const uint32_t &depth = 0);

    bool is_clicked(uint32_t id) const;

    void handle_events(const std::vector<SDL_Event> &events);

    // copy and swap idiom
    GUI(GUI&&) noexcept;
    GUI& operator=(GUI other);
    friend void swap(GUI &lhs, GUI &rhs);


    static GUIElement *VPadding(GUIElement *element, float padding);
    static GUIElement *HPadding(GUIElement *element, float padding);
    static GUIElement *Padding(GUIElement *element, float padding);
};