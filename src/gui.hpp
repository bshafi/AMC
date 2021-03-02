#pragma once

#include <vector>
#include <variant>
#include <map>

#include "shader.hpp"

union SDL_Event;

struct Image {
    // TODO: Change the rects into two vec2 it will make them easier to use
    glm::vec4 dest_rect;
    glm::vec4 source_rect;
    unsigned int depth;
};

struct Button {
    Image image;
    unsigned int button_id;
};

using GUIElement = std::variant<Button, Image>;

// TODO: Fix the coordinate system use (0,0) top left corner and (1,1) bottom right corner for gui coordinates
struct GUI {
    static constexpr uint32_t STRIDE = sizeof(glm::vec4) + sizeof(unsigned) + sizeof(glm::vec4) + sizeof(unsigned);
    Shader shader;

    unsigned int gui_VBO;
    unsigned int gui_VAO;
    unsigned int square_VBO;

    std::vector<Texture> textures;
    std::map<unsigned, std::pair<glm::vec4, bool>> buttons;
    glm::ivec2 window_size;

    bool is_clicked(const unsigned int button_id) const;

    void load_images(const pair_vector<std::string, GUIElement> &);

    GUI();
    ~GUI();

    GUI(const GUI&) = delete;
    GUI(GUI &&) = delete;

    void handle_events(const std::vector<SDL_Event> &event);
    void draw();
};