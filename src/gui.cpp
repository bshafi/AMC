#include <array>
#include <vector>
#include <iostream>

#include "gl_helper.hpp"

#include "gui.hpp"

constexpr std::array<float, 2 * 6> square_vertices = {
    0, 0,
    0, 1,
    1, 1,

    0, 0,
    1, 1,
    1, 0
};

GUI::GUI() : shader{ "shaders/gui.vert", "shaders/gui.frag"} {
    ASSERT_ON_GL_ERROR();

    glGenVertexArrays(1, &gui_VAO);
    glGenBuffers(1, &gui_VBO);
    glGenBuffers(1, &square_VBO);

    glBindVertexArray(gui_VAO);

    ASSERT_ON_GL_ERROR();

    glBindBuffer(GL_ARRAY_BUFFER, square_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(decltype(square_vertices)::value_type) * square_vertices.size(), square_vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);
    glEnableVertexAttribArray(0);

    ASSERT_ON_GL_ERROR();

    glBindBuffer(GL_ARRAY_BUFFER, gui_VBO);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, STRIDE, (void*)0);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(1);

    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, STRIDE, (void*)sizeof(glm::vec4));
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, STRIDE, (void*)(sizeof(glm::vec4) + sizeof(unsigned)));
    glVertexAttribDivisor(3, 1);
    glEnableVertexAttribArray(3);

    glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, STRIDE, (void*)(sizeof(glm::vec4) + sizeof(unsigned) + sizeof(glm::vec4)));
    glVertexAttribDivisor(4, 1);
    glEnableVertexAttribArray(4);

    ASSERT_ON_GL_ERROR();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    auto window = SDL_GL_GetCurrentWindow();
    assert(window);
    //SDL_GetWindowSize(window, &window_size.x, &window_size.y);
    SDL_GL_GetDrawableSize(window, &window_size.x, &window_size.y);
}
GUI::~GUI() {
    glDeleteBuffers(1, &gui_VBO);
    glDeleteBuffers(1, &square_VBO);
    glDeleteVertexArrays(1, &gui_VAO);
}

void GUI::load_images(const pair_vector<std::string, GUIElement> &image_args) {
    this->textures.clear();
    this->textures.reserve(image_args.size());

    ASSERT_ON_GL_ERROR();

    glBindBuffer(GL_ARRAY_BUFFER, gui_VBO);
    glBufferData(GL_ARRAY_BUFFER, STRIDE * image_args.size(), nullptr, GL_DYNAMIC_DRAW);

    unsigned i = 0;
    for (const auto &[texture_path, element] : image_args) {
        this->textures.emplace_back(texture_path);
        Image image;
        if (auto _image = std::get_if<Image>(&element)) {
            image = *_image;
        } else if (auto _button = std::get_if<Button>(&element)) {
            image = _button->image;
            buttons[_button->button_id] = { image.dest_rect, false };
        } else {
            assert(false);
        }

        const glm::vec4 dest_rect = image.dest_rect;
        const glm::vec4 source_rect = image.source_rect;
        const unsigned depth = image.depth;
        const unsigned texture_id = i;

        glBufferSubData(GL_ARRAY_BUFFER, (i * STRIDE), sizeof(glm::vec4), glm::value_ptr(dest_rect));
        glBufferSubData(GL_ARRAY_BUFFER, (i * STRIDE) + sizeof(glm::vec4), sizeof(unsigned), &texture_id);
        glBufferSubData(GL_ARRAY_BUFFER, (i * STRIDE) + sizeof(glm::vec4) + sizeof(unsigned), sizeof(source_rect), glm::value_ptr(source_rect));
        glBufferSubData(GL_ARRAY_BUFFER, (i * STRIDE) + sizeof(glm::vec4) + sizeof(unsigned) + sizeof(glm::vec4), sizeof(depth), &depth);
        
        ++i;
    }



    ASSERT_ON_GL_ERROR();
}

void GUI::handle_events(const std::vector<SDL_Event> &events) {
    for (const auto &event : events) {
        switch (event.type) {
        case SDL_MOUSEBUTTONUP:
            for (auto &[_0, button_state] : buttons) {
                button_state.second = false;
            }
            break;
        case SDL_MOUSEBUTTONDOWN: {
            // transform into opengl coordiates
            float x = (((static_cast<float>(event.button.x) / window_size.x)) * 4) - 1, y = ((0.5f - (static_cast<float>(event.button.y) / window_size.y)) * 4) - 1;
            for (auto &[_, button] : buttons) {
                auto &[rect, is_clicked] = button;
                if (rect.x <= x && x <= rect.x + rect.z && rect.y <= y && y <= rect.y + rect.w) {
                    is_clicked = true;
                }
            }
        }
        default:
            if (event.type == WINDOW_TRUE_RESIZE_EVENT) {
                intptr_t w = reinterpret_cast<intptr_t>(event.user.data1), h = reinterpret_cast<intptr_t>(event.user.data2);
                window_size.x = static_cast<int>(w);
                window_size.y = static_cast<int>(h);
            }
            break;
        }


    }
}

bool GUI::is_clicked(const unsigned int button_id) const {
    auto loc = buttons.find(button_id);
    assert(loc != buttons.cend());

    return loc->second.second;
}
void GUI::draw() {
    pair_vector<std::string, std::reference_wrapper<Texture>> bindings;
    bindings.reserve(this->textures.size());
    for (unsigned int i = 0; i < this->textures.size(); ++i) {
        bindings.push_back({ "textures[" + std::to_string(i) + "]", textures[i] });
    }
    shader.bind_texture_to_sampler_2D(bindings);

    shader.use();

    glBindVertexArray(gui_VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, square_vertices.size() / 2, this->textures.size());

    ASSERT_ON_GL_ERROR();
}