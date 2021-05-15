#include "gl_helper.hpp"
#include "gui_element_visitor.hpp"

#include "gui.hpp"
#include "tool.hpp"
#include "inventory.hpp"

#ifdef MANUAL_LEAK_CHECK
    std::map<uintptr_t, std::pair<std::string, uint32_t>> pointers = std::map<uintptr_t, std::pair<std::string, uint32_t>>();
#endif

const std::array<uint32_t, 6> corners = {
    0,
    1,
    2,

    0,
    1,
    3
};

uint32_t Renderer::corners_VBO = 0;
uint32_t Renderer::sprite_VAO = 0;
std::unique_ptr<Shader> Renderer::shader = nullptr;
std::unique_ptr<Texture> Renderer::tools = nullptr;
std::unique_ptr<Texture> Renderer::blocks = nullptr;

void Renderer::Initialize() {
    assert(shader == nullptr);
    assert(sprite_VAO == 0);
    assert(corners_VBO == 0);


    ASSERT_ON_GL_ERROR();

    glGenVertexArrays(1, &sprite_VAO);
    glGenBuffers(1, &corners_VBO);

    glBindVertexArray(sprite_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, corners_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(decltype(corners)::value_type) * corners.size(), corners.data(), GL_STATIC_DRAW);
    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(uint32_t), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    ASSERT_ON_GL_ERROR();


    shader = std::make_unique<Shader>("shaders/sprite.vert", "shaders/sprite.frag");
    tools = std::make_unique<Texture>("resources/tools.png");
    blocks = std::make_unique<Texture>("resources/blocks.png");
}
void Renderer::Terminate() {
    assert(shader != nullptr);
    assert(sprite_VAO != 0);
    assert(corners_VBO != 0);

    glDeleteBuffers(1, &corners_VBO);
    glDeleteVertexArrays(1, &sprite_VAO);

    corners_VBO = 0;
    sprite_VAO = 0;

    shader.reset();
    tools.reset();
    blocks.reset();


    assert(shader == nullptr);
    assert(sprite_VAO == 0);
    assert(corners_VBO == 0);
}

template <>
void Renderer::draw<Sprite>(const frect &dest, const uint32_t depth, const Sprite &sprite) {
    assert(shader != nullptr);
    assert(sprite_VAO != 0);
    assert(corners_VBO != 0);

    auto &texture = sprite.texture;
    auto &source = sprite.source;

    shader->apply_bindings({
        { "tex", texture }
    });
    shader->use();
    
    auto window_bounds = GetTrueWindowSize();

    auto h_source_rect = shader->retrieve_shader_variable<glm::vec4>("source_rect");
    shader->set(h_source_rect, static_cast<glm::vec4>(source));
    auto h_dest_rect = shader->retrieve_shader_variable<glm::vec4>("dest_rect");
    shader->set(h_dest_rect, static_cast<glm::vec4>(dest));
    auto h_depth = shader->retrieve_shader_variable<uint32_t>("depth");
    shader->set(h_depth, depth);
    auto h_window_bounds = shader->retrieve_shader_variable<glm::uvec2>("window_bounds");
    shader->set(h_window_bounds, window_bounds);
    auto h_texture_bounds = shader->retrieve_shader_variable<glm::uvec2>("texture_bounds");
    shader->set(h_texture_bounds, glm::uvec2(texture.width(), texture.height()));
    auto h_color = shader->retrieve_shader_variable<glm::vec4>("color");
    h_color.set(glm::vec4(0, 0, 0, 1));
    auto h_opacity = shader->retrieve_shader_variable<float>("opacity");
    h_opacity.set(1.f);
    auto h_color_mix = shader->retrieve_shader_variable<float>("color_mix");
    h_color_mix.set(0.f);


    glBindVertexArray(sprite_VAO);
    glDrawArrays(GL_TRIANGLES, 0, corners.size());
}

template <>
void Renderer::draw<color>(const frect &dest, const uint32_t depth, const color &color) {
    assert(shader != nullptr);
    assert(sprite_VAO != 0);
    assert(corners_VBO != 0);

    shader->use();
    
    auto window_bounds = GetTrueWindowSize();

    auto h_dest_rect = shader->retrieve_shader_variable<glm::vec4>("dest_rect");
    shader->set(h_dest_rect, static_cast<glm::vec4>(dest));
    
    auto h_window_bounds = shader->retrieve_shader_variable<glm::uvec2>("window_bounds");
    shader->set(h_window_bounds, window_bounds);
    
    auto h_color = shader->retrieve_shader_variable<glm::vec4>("color");
    shader->set(h_color, static_cast<glm::vec4>(color));
    
    auto h_opacity = shader->retrieve_shader_variable<float>("opacity");
    shader->set(h_opacity, 1.f);
    
    auto h_color_mix = shader->retrieve_shader_variable<float>("color_mix");
    shader->set(h_color_mix, 1.f);
    
    auto h_depth = shader->retrieve_shader_variable<uint32_t>("depth");
    shader->set(h_depth, depth);


    glBindVertexArray(sprite_VAO);
    glDrawArrays(GL_TRIANGLES, 0, corners.size());
}

template <>
void Renderer::draw(const frect &outer, const uint32_t depth, const BlockType &block_type) {
    Renderer::draw(outer, depth, Sprite{ *Renderer::blocks, BlockRect(block_type) });
}


GUIElement *GUI::VPadding(GUIElement *element, float h_padding) {
    assert(element);
    assert(0 <= h_padding && h_padding <= 1);

    const float padding = h_padding / 2.0f;

    const float other_padding = (1 - 2 * padding) / (1 - padding);
    GUIElement *inner = new VBisection(element, nullptr, other_padding);
    ADD_POINTER(inner);
    GUIElement *outer = new VBisection(nullptr, inner, padding);
    ADD_POINTER(outer);

    return outer;
}
GUIElement *GUI::HPadding(GUIElement *element, float h_padding) {
    assert(element);
    assert(0 <= h_padding && h_padding <= 1);
    
    const float padding = h_padding / 2;
    const float other_padding = (1 - 2 * padding) / (1 - padding);
    GUIElement *inner = new HBisection(element, nullptr, other_padding);
    ADD_POINTER(inner);
    GUIElement *outer = new HBisection( nullptr, inner, padding);
    ADD_POINTER(outer);

    return outer;
}
GUIElement *GUI::Padding(GUIElement *element, float padding) {
    return VPadding(HPadding(element, padding), padding);
}


GUIImage::GUIImage(Texture &texture, const frect &source, bool should_stretch)
    : texture{ texture }, source{ source }, should_stretch{ should_stretch } {

}
GUIImage::GUIImage(Texture &texture, bool should_stretch)
    : GUIImage(texture, frect{ 0, 0, static_cast<float>(texture.width()), static_cast<float>(texture.height()) }, should_stretch) {
    
}


void GUIImage::draw(const frect &outer, const uint32_t depth) {
    Renderer::draw(this->dest_rect(outer), depth, Sprite{ texture, source });
}
frect GUIImage::dest_rect(const frect &outer) const {
    frect dest_rect;
    if (this->should_stretch) {
        dest_rect = outer;
    } else {
        dest_rect = min_max_scaling(this->source, outer);
        std::cout << "";
    }

    return dest_rect;
}


ZStack::ZStack(const std::vector<GUIElement*> &layers)
    : layers{layers} {

}
ZStack::~ZStack() {
    for (auto element : layers) {
        REMOVE_POINTER(element);
        delete element;
    }
}

void ZStack::draw(const frect &outer, const uint32_t depth) {
    uint32_t i = 0;
    for (auto element : layers) {
        element->draw(this->dest_rect(outer), depth + i);
        ++i;
    }
}
void ZStack::handle_events(const SDL_Event &event, const frect &outer, const uint32_t depth) {
    uint32_t i = 0;
    for (auto element : layers) {
        element->handle_events(event, this->dest_rect(outer), depth + i);
        ++i;
    }
}
frect ZStack::dest_rect(const frect &outer) const {
    return outer;
}
void ZStack::visit(GUIElementVisitor &visitor) {
    visitor.visit(*this);
    for (auto element : layers) {
        element->visit(visitor);
    }
}
void ZStack::visit(ConstGUIElementVisitor &visitor) const {
    visitor.visit(*this);
    for (auto element : layers) {
        element->visit(visitor);
    }
}
ZStack& ZStack::operator=(ZStack rhs) {
    swap(*this, rhs);
    return *this;
}
ZStack::ZStack(ZStack &&rhs)
    : layers{} {
    swap(*this, rhs);
}
void swap(ZStack &rhs, ZStack &lhs) {
    using std::swap;

    swap(rhs.layers, lhs.layers);
}

Button::Button(Texture &texture, const frect &normal, const frect &on_hover, uint32_t id)
    : texture{ texture }, normal{ normal }, on_hover{ on_hover }, h_id{ id }, is_clicked{ false }, mouse_is_hovering{ false } {

}
void Button::draw(const frect &outer, const uint32_t depth) {
    Renderer::draw(dest_rect(outer), depth, Sprite{ texture, source_rect() });
}
void Button::handle_events(const SDL_Event &event, const frect &outer, const uint32_t depth) {
    const auto dest_rect = this->dest_rect(outer);
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        const glm::vec2 mouse_pos = glm::vec2(event.button.x, event.button.y);
        if (dest_rect.contains(mouse_pos)) {
            this->is_clicked = true;
        } else {
            this->is_clicked = false;
        }
    }
    if (event.type == SDL_MOUSEBUTTONUP) {
        this->is_clicked = false;
    }
    if (event.type == SDL_MOUSEMOTION) {
        if (dest_rect.contains(glm::vec2(event.motion.x, event.motion.y))) {
            this->mouse_is_hovering = true;
        } else {
            this->mouse_is_hovering = false;
        }
    }
}
frect Button::dest_rect(const frect &outer) const {
    frect source_rect = this->source_rect();
    const frect dest_rect = min_max_scaling(source_rect, outer);
    return dest_rect;
}
frect Button::source_rect() const {
    frect source_rect;
    if (mouse_is_hovering) {
        source_rect = this->on_hover;
    } else {
        source_rect = this->normal;
    }
    return source_rect;
}
bool Button::clicked() const {
    return is_clicked;
}
uint32_t Button::id() const {
    return h_id;
}
void Button::visit(GUIElementVisitor &visitor) {
    visitor.visit(*this);
}
void Button::visit(ConstGUIElementVisitor &visitor) const {
    visitor.visit(*this);
}

ButtonClickedVisitor::ButtonClickedVisitor(uint32_t id)
    : id{ id }, h_is_clicked{ false } {

}
void ButtonClickedVisitor::visit(const Button &button) {
    if (button.id() == this->id && button.clicked()) {
        h_is_clicked = true;
    }
}

bool ButtonClickedVisitor::is_clicked() const {
    return h_is_clicked;
}

HBisection::HBisection(GUIElement *left, GUIElement *right, float top_section)
    : top{ left }, bottom{ right }, top_section{ top_section } {
    assert(0 <= top_section && top_section <= 1.0f);
}
HBisection::~HBisection() {
    if (top != nullptr) {
        REMOVE_POINTER(top);
        delete top;
    }
    if (bottom != nullptr) {
        REMOVE_POINTER(bottom);
        delete bottom;
    }
}

void HBisection::draw(const frect &outer, const uint32_t depth) {
    if (this->top != nullptr) {
        top->draw(this->top_rect(outer), depth);
    }
    if (this->bottom != nullptr) {
        bottom->draw(this->bottom_rect(outer), depth);
    }
}
void HBisection::handle_events(const SDL_Event &event, const frect &outer, const uint32_t depth) {
    if (this->top != nullptr) {
        top->handle_events(event, top_rect(outer), depth);
    } 
    if (this->bottom != nullptr) {
        bottom->handle_events(event, bottom_rect(outer), depth);
    }
}
frect HBisection::top_rect(const frect &outer) const {
    assert(0 <= top_section && top_section <= 1.0f);
    return frect{ outer.x, outer.y , outer.w, outer.h * top_section };
}
frect HBisection::bottom_rect(const frect &outer) const {
    assert(0 <= top_section && top_section <= 1.0f);
    return frect{ outer.x, outer.y + outer.h * top_section, outer.w, outer.h * (1 - top_section) };
}
frect HBisection::dest_rect(const frect &outer) const {
    return outer;
}
void HBisection::visit(GUIElementVisitor &visitor) {
    visitor.visit(*this);
    if (this->top != nullptr) {
        this->top->visit(visitor);
    }
    if (this->bottom != nullptr) {
        this->bottom->visit(visitor);
    }
}
void HBisection::visit(ConstGUIElementVisitor &visitor) const {
    visitor.visit(*this);
    if (this->top != nullptr) {
        this->top->visit(visitor);
    }
    if (this->bottom != nullptr) {
        this->bottom->visit(visitor);
    }
}

HBisection& HBisection::operator=(HBisection rhs) {
    swap(*this, rhs);
    return *this;
}
HBisection::HBisection(HBisection &&rhs)
    : top{ nullptr }, bottom{ nullptr }, top_section{-std::numeric_limits<float>::infinity() } {
    swap(*this, rhs);
}
void swap(HBisection &lhs, HBisection &rhs) {
    using std::swap;

    swap(lhs.top, rhs.top);
    swap(lhs.bottom, rhs.bottom);
    swap(lhs.top_section, rhs.top_section);
}


VBisection::VBisection(GUIElement *left, GUIElement *right, float left_section)
    : left{ left }, right{ right }, left_section{ left_section } {
    assert(0 <= left_section && left_section <= 1);
}
VBisection::~VBisection() {
    if (left != nullptr) {
        REMOVE_POINTER(left);
        delete left;
    }
    if (right != nullptr) {
        REMOVE_POINTER(right);
        delete right;
    }
}

void VBisection::draw(const frect &outer, const uint32_t depth) {
    if (left != nullptr) {
        left->draw(left_rect(outer), depth);
    }
    if (right != nullptr) {
        right->draw(right_rect(outer), depth);
    }
}
void VBisection::handle_events(const SDL_Event &event, const frect &outer, const uint32_t depth) {
    if (left != nullptr) {
        left->handle_events(event, left_rect(outer), depth);
    }
    if (right != nullptr) {
        right->handle_events(event, right_rect(outer), depth);
    }
}
frect VBisection::dest_rect(const frect &outer) const {
    return outer;
}
void VBisection::visit(GUIElementVisitor &visitor) {
    visitor.visit(*this);
    if (left != nullptr) {
        left->visit(visitor);
    }
    if (right != nullptr) {
        right->visit(visitor);
    }
}
void VBisection::visit(ConstGUIElementVisitor &visitor) const {
    visitor.visit(*this);
    if (left != nullptr) {
        left->visit(visitor);
    }
    if (right != nullptr) {
        right->visit(visitor);
    }
}

frect VBisection::left_rect(const frect &outer) const {
    assert(0 <= left_section && left_section <= 1);
    return frect{ outer.x, outer.y, outer.w * left_section, outer.h };
}
frect VBisection::right_rect(const frect &outer) const {
    assert(0 <= left_section && left_section <= 1);
    return frect{ outer.x + outer.w * left_section, outer.y, outer.w * (1 - left_section), outer.h };
}

VBisection& VBisection::operator=(VBisection vbisection) {
    swap(*this, vbisection);
    return *this;
}
VBisection::VBisection(VBisection &&rhs)
    :  left{ nullptr }, right{ nullptr }, left_section{ -std::numeric_limits<float>::infinity() } {
    swap(*this, rhs);
}
void swap(VBisection &lhs, VBisection &rhs) {
    using std::swap;

    swap(lhs.left, rhs.left);
    swap(lhs.right, rhs.right);
    swap(lhs.left_section, rhs.left_section);
}

void GUI::handle_events(const std::vector<SDL_Event> &events) {
    assert(root);

    for (const auto &event : events) {
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
            window_bounds.x = static_cast<uint32_t>(event.window.data1);
            window_bounds.y = static_cast<uint32_t>(event.window.data2);
        }
        root->handle_events(event, frect{ 0, 0, static_cast<float>(window_bounds.x), static_cast<float>(window_bounds.y) }, 0);
    }
}

bool GUI::is_clicked(uint32_t id) const {
    ButtonClickedVisitor bcv(id);

    root->visit(bcv);

    return bcv.is_clicked();
}

void GUI::draw(const uint32_t &depth) {
    frect outer = {
        0,
        0,
        static_cast<float>(window_bounds.x),
        static_cast<float>(window_bounds.y)
    };
    assert(root);
    root->draw(outer, 0);
}

GUI::GUI()
    : root{ nullptr } {
    
    window_bounds = GetTrueWindowSize();
    SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &fake_window_bounds.x, &fake_window_bounds.y);
}
GUI::~GUI() {

    REMOVE_POINTER(root);
    delete root;
    for (auto texture : textures) {
        REMOVE_POINTER(texture);
        delete texture;
    }
}

GUI::GUI(GUI &&other) noexcept
    : textures{}, root{ nullptr } {
    swap(*this, other);
}
GUI& GUI::operator=(GUI other) {
    swap(*this, other);
    return *this;
}
void swap(GUI &lhs, GUI &rhs) {
    using std::swap;
    swap(lhs.root, rhs.root);
    swap(lhs.textures, rhs.textures);
}

void GUI::set_textures(std::vector<Texture*> &textures) {
    // only can be set once
    assert(this->textures.empty());

    std::swap(this->textures, textures);
}
void GUI::set_root(GUIElement **root) {
    // only can be set once
    assert(this->root == nullptr);
    assert(root);

    this->root = *root;
    *root = nullptr;
}