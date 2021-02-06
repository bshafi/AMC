#pragma once

#include <glm/glm.hpp>

#include "shader.hpp"
#include "standard.hpp"

enum BlockType : uint32_t {
    Air = 0,
    Grass = 1,
    Dirt = 2,
    Stone = 3
};

union SDL_Event;
/*
    A 16x x 16z x 255y Chunk of blocks
    used to store block ids and other data that pertains to a block
 */
struct Chunk {
    static const unsigned BLOCKS_IN_CHUNK = 256 * 16 * 16;
    static const unsigned CHUNK_WIDTH = 16;
    static const unsigned CHUNK_HEIGHT = 256;
    using BlockIDType = unsigned;

    std::array<BlockIDType, CHUNK_HEIGHT * CHUNK_WIDTH * CHUNK_WIDTH> blocks;
    glm::ivec2 chunk_pos;

    BlockIDType GetBlock(glm::ivec3 pos) const;
    void SetBlock(glm::ivec3 pos, BlockIDType id);


    static const int32_t MIN_X = 0, MAX_X = 15, MIN_Y = 0, MAX_Y = 255, MIN_Z = 0, MAX_Z = 15;
    static uint32_t world_pos_to_index(glm::ivec3 pos);
    // TODO: better name
    // use in a for loop like for(auto pos = glm::ivec3(); loop_through(pos);)
    static void loop_through(glm::ivec3 &pos);
    static bool is_within_chunk_bounds(const glm::ivec3 &pos);

    bool intersects(glm::vec3 pos, AABB aabb) const;

    Chunk();
    ~Chunk();
};