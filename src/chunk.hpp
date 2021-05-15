#pragma once

#include <glm/glm.hpp>

#include "shader.hpp"
#include "standard.hpp"

enum class BlockType : uint32_t {
    BLOCK_MIN = 0,
    Air = 0,
    Grass = 1,
    Dirt = 2,
    Stone = 3,
    BLOCK_MAX
};
constexpr uint32_t BLOCK_MIN = static_cast<uint32_t>(BlockType::BLOCK_MIN);
constexpr uint32_t BLOCK_MAX = static_cast<uint32_t>(BlockType::BLOCK_MAX);



frect BlockRect(const BlockType &type);

union SDL_Event;
/*
    A 16x x 16z x 255y Chunk of blocks
    used to store block ids and other data that pertains to a block
 */
struct Chunk {
    static const uint32_t WIDTH = 16;
    static const uint32_t HEIGHT = 256;
    static const uint32_t LENGTH = WIDTH;
    static const uint32_t BLOCKS_IN_CHUNK = WIDTH * HEIGHT * LENGTH;


    Array3d<WIDTH, HEIGHT, LENGTH, BlockType> blocks;
    glm::ivec2 chunk_pos;

    BlockType GetBlock(glm::ivec3 pos) const;
    void SetBlock(glm::ivec3 pos, BlockType id);


    static const int32_t MIN_X = 0, MAX_X = 15, MIN_Y = 0, MAX_Y = 255, MIN_Z = 0, MAX_Z = 15;
    static uint32_t world_pos_to_index(glm::ivec3 pos);
    // TODO: better name
    // use in a for loop like for(auto pos = glm::ivec3(0, 0, 0); loop_through(pos);)
    static void loop_through(glm::ivec3 &pos);
    static bool is_within_chunk_bounds(const glm::ivec3 &pos);


    bool intersects(glm::vec3 pos, AABB aabb) const;

    Chunk();
    ~Chunk();
};