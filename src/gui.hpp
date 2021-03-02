#pragma once

#include "standard.hpp"
#include "shader.hpp"

union SDL_Event;

struct Renderer {
    uint32_t corners_VBO, sprite_VAO;
    Shader shader;

    void draw(const Texture&, const frect &source, const frect &dest, const uint32_t &depth = 0);

    Renderer(Renderer &&other) noexcept;
    Renderer& operator=(Renderer other);
    friend void swap(Renderer &lhs, Renderer &rhs);

    Renderer();
    ~Renderer();
};

struct GUIElement;

struct VBisection {
    GUIElement *left = nullptr, *right = nullptr;
    float left_section = 0.5f;
};
struct HBisection {
    GUIElement *top = nullptr, *bottom = nullptr;
    float top_section = 0.5f;
};
struct Button {
    Texture *texture = nullptr;
    frect normal, on_hover;
    uint32_t id = 0;
    bool is_clicked = false;
    bool mouse_is_hovering = false;
};

struct Sprite {
    Texture *texture = nullptr;
    frect source;
    bool should_stretch;

    Sprite(Texture *texture, const frect &source, bool should_stretch = false);
    Sprite(Texture *texture, bool should_stretch = false);
};

struct ZStack {
    GUIElement ** elements;
    uint32_t size;
};

enum class GUIElementType {
    VBisection,
    HBisection,
    Sprite,
    ZStack,
    Button
};

struct GUIElement {
    GUIElementType type;
    union {
        VBisection vbisection;
        HBisection hbisection;
        Sprite sprite;
        ZStack zstack;
        Button button;
    };
    // takes ownership of vbisection
    GUIElement(const VBisection &vbisection);
    GUIElement(const HBisection &hbisection);
    GUIElement(const Sprite &sprite);
    // All the elements should be dynamically allocated
    GUIElement(const std::vector<GUIElement*> &elements);
    GUIElement(const Button &button);

    GUIElement(const GUIElement &) = delete;
    GUIElement& operator=(GUIElement &) = delete;

    ~GUIElement();

};

struct GUI {
    std::vector<Texture*> textures;
    GUIElement *root;
    glm::uvec2 window_bounds;
    glm::ivec2 fake_window_bounds;

    GUI();
    ~GUI();

    using Visitor = std::function<void(GUIElement *, Texture&, const frect&, const frect&, const uint32_t&)>;

    // Takes ownership both of these should only be called once
    void set_textures(std::vector<Texture*> &);
    void set_root(GUIElement **root);

    void apply_to_leaves(Visitor, GUIElement *element, const frect &outer, const uint32_t &depth);
    void draw(Renderer &, const uint32_t &depth = 0);
    void draw(Renderer &, GUIElement *element, const frect &outer, const uint32_t &depth = 0);

    void handle_events(const std::vector<SDL_Event> &events);

    // copy and swap idiom
    GUI(GUI&&) noexcept;
    GUI& operator=(GUI other);
    friend void swap(GUI &lhs, GUI &rhs);


    static GUIElement *VPadding(GUIElement *element, float padding);
    static GUIElement *HPadding(GUIElement *element, float padding);
};