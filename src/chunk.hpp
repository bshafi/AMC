#pragma once

#include <limits>

#include "shader.hpp"
#include "standard.hpp"

enum class BlockType : uint32_t {
    Air = 0,
    Grass = 1,
    Dirt = 2,
    Stone = 3,
    Sand = 4,
    Wood = 5,
    Leaves = 6,
};

enum class Biome : uint8_t {
    Unkown = 0,
    Plains = 1,
    Dessert = 2,
    ocean = 3,
};

frect BlockRect(const BlockType &type);

union SDL_Event;
/*
    A 16x x 16z x 255y Chunk of blocks
    used to store block ids and other data that pertains to a block
 */
struct Chunk {
public:
    static const unsigned BLOCKS_IN_CHUNK = 256 * 16 * 16;
    static const unsigned CHUNK_WIDTH = 16;
    static const unsigned CHUNK_HEIGHT = 256;

    // std::array<BlockType, CHUNK_HEIGHT * CHUNK_WIDTH * CHUNK_WIDTH> blocks;
    Array3d<CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_WIDTH, BlockType> blocks;
    glm::ivec2 chunk_pos;

    Chunk();
    ~Chunk();

    const BlockType &GetBlock(glm::ivec3 pos) const;
    BlockType &GetBlock(glm::ivec3 pos);
    //void SetBlock(glm::ivec3 pos, BlockType id);
    BlockType GetBlockFromWorld(glm::ivec3 pos) const;

    //static Chunk generate(const Biome current, const glm::ivec2 chunk_pos);

    static const int32_t MIN_X = 0, MAX_X = 15, MIN_Y = 0, MAX_Y = 255, MIN_Z = 0, MAX_Z = 15;
    static uint32_t world_pos_to_index(glm::ivec3 pos);
    // TODO: better name
    // use in a for loop like for(auto pos = glm::ivec3(); loop_through(pos);)
    static void loop_through(glm::ivec3 &pos);
    static bool is_within_chunk_bounds(const glm::ivec3 &pos);

    bool intersects(glm::vec3 pos, AABB aabb) const;

    AABB bounding_box() const;
    glm::vec3 world_pos() const;
    static glm::vec3 chunk_pos_to_world_pos(glm::ivec2);
    static glm::ivec2 world_pos_to_chunk_pos(glm::vec3);
};

bool chunk_contains(const Chunk &chunk, const glm::ivec3 &pos);
bool min_nonegative(float a, float b);
glm::ivec3 get_hit_block(const Chunk &chunk, const glm::vec3 &dir, const glm::vec3 &hit_pos);

struct BlockHit {
    glm::ivec2 chunk_pos = { 0, 0 };
    glm::ivec3 block_pos = { 0, 0, 0 };
    enum class Face {
        Top = 0,
        PosY = Top,
        Bottom = 1,
        NegY = Bottom,
        Right = 2,
        PosX = Right,
        Left = 3,
        NegX = Left,
        Front = 4,
        PosZ = Front,
        Back = 5,
        NegZ = Back
    };
    // Normal of the face of the block that was hit
    Face face;
};

const char *get_face_name(BlockHit::Face face);
constexpr const char *face_names[] = {
    "PosY",
    "NegY",
    "PosX",
    "NegX",
    "PosZ",
    "NegZ"
};