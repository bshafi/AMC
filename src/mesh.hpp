#pragma once

#include <standard.hpp>
#include "chunk.hpp"

struct Chunk;

struct BlockVertex {
    glm::vec3 position;
    glm::vec2 tex_coords;
    BlockType block_id;
};

struct BlockMesh {
    std::vector<BlockVertex> vertices;
    static BlockMesh Generate(const Chunk &chunk);
};