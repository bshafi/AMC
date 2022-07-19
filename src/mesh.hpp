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

struct Vertex {
    vec3 vertex_position;
    vec2 texture_coordinate;
    uint32_t texture_id;

    static constexpr uint32_t gl_size = sizeof(vec3) + sizeof(vec2) + sizeof(uint32_t);
};



struct Mesh {
    std::vector<Vertex> vertices;
};


Mesh CreateCubeMesh(glm::vec3 size = { 1.f, 1.f, 1.f }, glm::vec3 pos = { 0.f, 0.f, 0.f }, std::array<glm::vec4, 6> faces = {});

class MeshBuffer {
public:
    MeshBuffer(MeshBuffer &&rhs) noexcept;
    MeshBuffer& operator=(MeshBuffer rhs) noexcept;
    friend void swap(MeshBuffer &lhs, MeshBuffer &rhs) noexcept;

    void rebuild(const Mesh &mesh);
    void rebuild(const Chunk &);

    void draw();
    MeshBuffer(const Chunk&);
    MeshBuffer(const BlockMesh &block_mess);
    MeshBuffer(const Mesh &mesh);
    MeshBuffer(uint32_t size);
    ~MeshBuffer();
private:
    uint32_t vbo, vao;
    uint32_t size;
    uint32_t capacity;
};