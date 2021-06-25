#pragma once

#include "standard.hpp"
#include "chunk.hpp"

struct Chunk;

struct BlockVertex {
    glm::vec3 position;
    glm::vec2 tex_coords;
    BlockType block_id;
    static constexpr uint32_t gl_size = sizeof(position) + sizeof(tex_coords) + sizeof(block_id);
};

struct BlockMesh {
    std::vector<BlockVertex> vertices;
    static BlockMesh Generate(const Chunk &chunk);
};

class MeshBuffer {
public:
    MeshBuffer(MeshBuffer &&rhs) noexcept;
    MeshBuffer& operator=(MeshBuffer rhs) noexcept;
    friend void swap(MeshBuffer &lhs, MeshBuffer &rhs) noexcept;

    void draw();
    MeshBuffer(const BlockMesh &block_mess);
    MeshBuffer(uint32_t size);
    ~MeshBuffer();
private:
    uint32_t vbo, vao;
    uint32_t size;
    uint32_t capacity;
};