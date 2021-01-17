#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <optional>
#include <map>

#include <glm/glm.hpp>

#include "shader.hpp"
#include "standard.hpp"

union SDL_Event;
/*
    A 16x x 16z x 255y Chunk of blocks
    used to store block ids and other data that pertains to a block
 */
struct Chunk {
    static constexpr uint32_t Width = 16;
    
    // [z][x]
    using Slice = std::array<std::array<unsigned, Width>, Width>;
    
    // slices[z][x][y]
    std::vector<Slice> slices;
    Uniform<glm::ivec3> chunk_location;
    Uniform<glm::mat4x4> view;
    Uniform<glm::mat4x4> projection;
    Uniform<float> aspect_ratio;
    glm::vec3 camera_pos;
    glm::vec3 forward;
    unsigned VAO, block_ids_VBO;
    float pitch, yaw;

    Chunk(uint32_t height);
    void draw();
    void update(const std::vector<SDL_Event> &events);

    static std::optional<unsigned> cube_VBO;
    static std::optional<Shader> cube_shader_program;
    static std::optional<Texture> cube_texture;
};
