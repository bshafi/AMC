#pragma once

#include "standard.hpp"
#include "chunk.hpp"

struct Chunk;

struct BlockVertex {
    glm::vec3 position;
    glm::vec2 tex_coords;
    BlockType block_id;
    glm::ivec3 original_location;
    static constexpr uint32_t gl_size = sizeof(position) + sizeof(tex_coords) + sizeof(block_id) + sizeof(original_location);
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

    void rebuild(const BlockMesh &block_mesh);

    void draw();
    MeshBuffer(const Chunk&);
    MeshBuffer(uint32_t size);
    ~MeshBuffer();
private:
    uint32_t vbo, vao;
    uint32_t size;
    uint32_t capacity;
};