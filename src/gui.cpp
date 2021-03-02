#include "gl_helper.hpp"

#include "gui.hpp"

const std::array<uint32_t, 6> corners = {
    0,
    1,
    2,

    0,
    1,
    3
};

Renderer::Renderer()
    : shader("shaders/sprite.vert", "shaders/sprite.frag") {

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
}
Renderer::~Renderer() {
    glDeleteBuffers(1, &corners_VBO);
    glDeleteVertexArrays(1, &sprite_VAO);
}
Renderer::Renderer(Renderer &&other) noexcept
    : corners_VBO{ 0 }, sprite_VAO{ 0 }, shader{ std::move(other.shader) } {
    swap(*this, other);
    this->shader = std::move(other.shader);
}
Renderer& Renderer::operator=(Renderer other) {
    swap(*this, other);
    return *this;
}
void swap(Renderer &lhs, Renderer &rhs) {
    using std::swap;
    swap(lhs.corners_VBO, rhs.corners_VBO);
    swap(lhs.sprite_VAO, rhs.sprite_VAO);
}

void Renderer::draw(const Texture &texture, const frect &source, const frect &dest, const uint32_t &depth) {
    shader.bind_texture_to_sampler_2D({
        { "tex", texture }
    });
    shader.use();
    
    auto window_bounds = GetTrueWindowSize();

    auto h_source_rect = shader.retrieve_shader_variable<glm::vec4>("source_rect");
    shader.set(h_source_rect, static_cast<glm::vec4>(source));
    auto h_dest_rect = shader.retrieve_shader_variable<glm::vec4>("dest_rect");
    shader.set(h_dest_rect, static_cast<glm::vec4>(dest));
    auto h_depth = shader.retrieve_shader_variable<uint32_t>("depth");
    shader.set(h_depth, depth);
    auto h_window_bounds = shader.retrieve_shader_variable<glm::uvec2>("window_bounds");
    shader.set(h_window_bounds, window_bounds);
    auto h_texture_bounds = shader.retrieve_shader_variable<glm::uvec2>("texture_bounds");
    shader.set(h_texture_bounds, glm::uvec2(texture.width(), texture.height()));


    glBindVertexArray(this->sprite_VAO);
    glDrawArrays(GL_TRIANGLES, 0, corners.size());
}


Sprite::Sprite(Texture *texture, const frect &source, bool should_stretch) {
    assert(texture);

    this->texture = texture;
    this->source = source;
    this->should_stretch = should_stretch;
}
Sprite::Sprite(Texture *texture, bool should_stretch) {
    assert(texture);

    this->texture = texture;
    this->source = frect{
        0,
        0,
        static_cast<float>(texture->width()),
        static_cast<float>(texture->height()),
    };
    this->should_stretch = should_stretch;
}

GUIElement::GUIElement(const VBisection &vbisection) {
    this->type = GUIElementType::VBisection;
    this->vbisection = vbisection;
}
GUIElement::GUIElement(const HBisection &hbisection) {
    this->type = GUIElementType::HBisection;
    this->hbisection = hbisection;
}
GUIElement::GUIElement(const Sprite &sprite) {
    this->type = GUIElementType::Sprite;
    this->sprite = sprite;
}
GUIElement::GUIElement(const std::vector<GUIElement*> &elements) {
    this->type = GUIElementType::ZStack;
    this->zstack = ZStack{
        new GUIElement*[elements.size()],
        static_cast<uint32_t>(elements.size())
    };
    for (uint32_t i = 0; i < elements.size(); ++i) {
        this->zstack.elements[i] = elements[i];
    }
}
GUIElement::GUIElement(const Button &button) {
    this->type = GUIElementType::Button;
    this->button = button;
}

GUIElement::~GUIElement() {
    switch (this->type) {
    case GUIElementType::VBisection:
        if (vbisection.left != nullptr) {
            delete vbisection.left;
        }
        if (vbisection.right != nullptr) {
            delete vbisection.right;
        }
        break;
    case GUIElementType::HBisection:
        if (hbisection.top != nullptr) {
            delete hbisection.top;
        }
        if (hbisection.bottom != nullptr) {
            delete hbisection.bottom;
        }
        break;
    case GUIElementType::ZStack:
        for (uint32_t i = 0; i < zstack.size; ++i) {
            delete zstack.elements[i];
        }
        delete [] zstack.elements;
        break;
    case GUIElementType::Sprite:
        break;
    case GUIElementType::Button:
        break;
    }
}


GUIElement *GUI::VPadding(GUIElement *element, float padding) {
    assert(element);
    assert(0 <= padding && padding <= 1);

    const float other_padding = (1 - 2 * padding) / (1 - padding);
    return new GUIElement(VBisection{ nullptr, new GUIElement(VBisection{ element, nullptr, other_padding }), padding});
}
GUIElement *GUI::HPadding(GUIElement *element, float padding) {
    assert(element);
    assert(0 <= padding && padding <= 1);

    const float other_padding = (1 - 2 * padding) / (1 - padding);
    return new GUIElement(HBisection{ nullptr, new GUIElement(HBisection{ element, nullptr, other_padding }), padding});
}

void GUI::handle_events(const std::vector<SDL_Event> &events) {
    for (const auto &event : events) {
        if (event.type == WINDOW_TRUE_RESIZE_EVENT) {
            window_bounds = GetWindowBoundsFromTrueResizeEvent(event);
        }
        if (event.type == SDL_WINDOWEVENT_RESIZED) {
            fake_window_bounds = { event.window.data1, event.window.data2 };
        }
        if (event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
            glm::vec2 mouse_pos;
            if (event.type == SDL_MOUSEMOTION) {
                mouse_pos = glm::vec2(event.motion.x, event.motion.y);
            } else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
                mouse_pos = glm::vec2(event.button.x, event.button.y);
            } else {
                assert(false);
            }
            const glm::vec2 relative_mouse_pos =  mouse_pos / glm::vec2(fake_window_bounds);
            
            apply_to_leaves(
                [&](GUIElement *element,Texture&, const frect &source, const frect &dest, const uint32_t&) {
                    
                    glm::vec4 dest_rect = static_cast<glm::vec4>(dest) / glm::vec4(window_bounds, window_bounds);
                    fflush(stdout);
                    if (element->type != GUIElementType::Button) {
                        return;
                    }

                    if (
                        dest_rect.x <= relative_mouse_pos.x && relative_mouse_pos.x <= dest_rect.x + dest_rect.z &&
                        dest_rect.y <= relative_mouse_pos.y && relative_mouse_pos.y <= dest_rect.y + dest_rect.w
                    ) {
                        element->button.mouse_is_hovering = true;
                        if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
                            element->button.is_clicked = (event.type == SDL_MOUSEBUTTONDOWN);
                        }
                    } else {
                        element->button.mouse_is_hovering = false;
                    }
                    
                },
                root,
                frect{ 0, 0, static_cast<float>(window_bounds.x), static_cast<float>(window_bounds.y) },
                0
            );
        }
    }
}

frect min_max_scaling(const frect &inner, const frect &outer) {
    float scale = std::min(outer.w / inner.w, outer.h / inner.h);
    float new_width = inner.w * scale;
    float new_height = inner.h * scale;
    return frect {
        outer.x + (outer.w - new_width) / 2.f,
        outer.y + (outer.h - new_height) / 2.f,
        new_width,
        new_height
    };
}

void GUI::draw(Renderer &renderer, const uint32_t &depth) {
    frect outer = {
        0,
        0,
        static_cast<float>(window_bounds.x),
        static_cast<float>(window_bounds.y)
    };
    draw(renderer, root, outer, depth);
}

void GUI::apply_to_leaves(GUI::Visitor visitor, GUIElement *element, const frect &outer, const uint32_t &depth) {
    switch (element->type) {
    case GUIElementType::VBisection: {
        const float left_section = element->vbisection.left_section;
        const frect left = { outer.x, outer.y, outer.w * left_section, outer.h };
        if (element->vbisection.left != nullptr) {
            apply_to_leaves(visitor, element->vbisection.left, left, depth);
        }
        const frect right = { outer.x + outer.w * left_section, outer.y, outer.w * (1 - left_section), outer.h };
        if (element->vbisection.right != nullptr) {
            apply_to_leaves(visitor, element->vbisection.right, right, depth);
        }
    }
        break;
    case GUIElementType::HBisection: {
        const float top_section = element->hbisection.top_section;
        const frect top = { outer.x, outer.y, outer.w, outer.h * (1 - top_section) };
        const frect bottom  = { outer.x, outer.y + outer.h * (1 - top_section), outer.w, outer.h * top_section };
        if (element->hbisection.top != nullptr) {
            apply_to_leaves(visitor, element->hbisection.top, top, depth);
        }
        if (element->hbisection.bottom != nullptr) {
            apply_to_leaves(visitor, element->hbisection.bottom, bottom, depth);
        }
    }
        break;
    case GUIElementType::Sprite: {
        frect dest_rect;
        if (element->sprite.should_stretch) {
            dest_rect = outer;
        } else {
            dest_rect = min_max_scaling(element->sprite.source, outer);
        }
        
        assert(element->sprite.texture);
        // Render here
        visitor(element, *element->sprite.texture, element->sprite.source, dest_rect, depth);
    }
        break;
    case GUIElementType::ZStack: {
        for (uint32_t i = 0; i < element->zstack.size; ++i) {
            apply_to_leaves(visitor, element->zstack.elements[i], outer, depth + i);
        }
    }
        break;
    case GUIElementType::Button: {
        frect source_rect;
        if (element->button.mouse_is_hovering) {
            source_rect = element->button.on_hover;
        } else {
            source_rect = element->button.normal;
        }
        const frect dest_rect = min_max_scaling(source_rect, outer);
        assert(element->button.texture);
        visitor(element, *element->button.texture, source_rect, dest_rect, depth);
    }
        break;
    }
}

void GUI::draw(Renderer &renderer, GUIElement *element, const frect &outer, const uint32_t &depth) {
    assert(element);
    apply_to_leaves(
        [&](GUIElement *, Texture &htexture, const frect &hsource, const frect &hdest, const uint32_t &hdepth){
            renderer.draw(htexture, hsource, hdest, hdepth);
        },
        root,
        outer,
        depth
    );
}

GUI::GUI()
    : root{ nullptr } {
    
    window_bounds = GetTrueWindowSize();
    SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &fake_window_bounds.x, &fake_window_bounds.y);
}
GUI::~GUI() {
    delete root;
    for (auto texture : textures) {
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
    assert(this->textures.empty());

    std::swap(this->textures, textures);
}
void GUI::set_root(GUIElement **root) {
    assert(this->root == nullptr);
    assert(root);

    this->root = *root;
    *root = nullptr;
}