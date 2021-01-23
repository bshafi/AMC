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
    static constexpr unsigned BLOCKS_IN_CHUNK = 256 * 16 * 16;
    using BlockIDType = unsigned;

    unsigned int cube_vertices_VBO, VAO, chunk_block_ids_VBO;
    Texture orientation_texture;
    Texture blocks_texture;
    Shader shader;


    Chunk();
    ~Chunk();

    Chunk(const Chunk &) = delete;
    Chunk(Chunk &&) = delete;

    void draw();
};