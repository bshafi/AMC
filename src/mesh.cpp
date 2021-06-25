#include "gl_helper.hpp"

#include "hello_cube.hpp"
#include "chunk.hpp"
#include "mesh.hpp"

BlockMesh BlockMesh::Generate(const Chunk &chunk) {
    BlockMesh mesh;
    
    const glm::ivec3 deltas[] = {
        glm::ivec3(-1, 0, 0), 
        glm::ivec3(1, 0, 0), 
        glm::ivec3(0, -1, 0), 
        glm::ivec3(0, 1, 0), 
        glm::ivec3(0, 0, -1), 
        glm::ivec3(0, 0, 1)
    };
    for (auto pos = glm::ivec3(); Chunk::is_within_chunk_bounds(pos); Chunk::loop_through(pos)) {
        BlockType block_type = chunk.GetBlock(pos);
        if (block_type == BlockType::Air) {
            continue;
        }
        bool is_surrounded = true;
        for (uint32_t i = 0; i < 6; ++i) {
            auto neighbor_pos = pos + deltas[i];
            if (!Chunk::is_within_chunk_bounds(neighbor_pos)) {
                is_surrounded = false;
                break;
            }
            is_surrounded &= chunk.GetBlock(neighbor_pos) != BlockType::Air;
        }
        if (is_surrounded) {
            continue;
        }
        const glm::vec3 center = glm::vec3(pos) + glm::vec3(0.5f, 0.5f, 0.5f);

        for (uint32_t i = 0; i < 36; ++i) {
            const uint32_t offset = i * 5;
            BlockVertex vertex = {
                center + glm::vec3(cube_vertices[offset + 0], cube_vertices[offset + 1], cube_vertices[offset + 2]),
                glm::vec2(cube_vertices[offset + 3], cube_vertices[offset + 4]),
                block_type
            };
            mesh.vertices.push_back(vertex);
        }
    }
    return mesh;
}


MeshBuffer::MeshBuffer(MeshBuffer &&rhs) noexcept
    : vbo(0), vao(0), size(0), capacity(0) {
    swap(*this, rhs);
}
MeshBuffer& MeshBuffer::operator=(MeshBuffer rhs) noexcept {
    swap(*this, rhs);
    return *this;
}
void swap(MeshBuffer &lhs, MeshBuffer &rhs) noexcept {
    using std::swap;

    swap(lhs.vbo, rhs.vbo);
    swap(lhs.vao, rhs.vao);
    swap(lhs.size, rhs.size);
    swap(lhs.capacity, rhs.capacity);
}
void MeshBuffer::draw() {
    ASSERT_ON_GL_ERROR();

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo); ASSERT_ON_GL_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, size); ASSERT_ON_GL_ERROR();

    glBindVertexArray(0); ASSERT_ON_GL_ERROR();
    glBindBuffer(GL_ARRAY_BUFFER, 0); ASSERT_ON_GL_ERROR();

    ASSERT_ON_GL_ERROR();
}
MeshBuffer::MeshBuffer(const BlockMesh &block_mesh)
    :  MeshBuffer(block_mesh.vertices.size()) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    for (uint32_t i = 0; i < block_mesh.vertices.size(); ++i) {
        const auto vertex = block_mesh.vertices[i];
        const uint32_t offset = i * BlockVertex::gl_size;
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(glm::vec3), glm::value_ptr(vertex.position));
        glBufferSubData(GL_ARRAY_BUFFER, offset + sizeof(glm::vec3), sizeof(glm::vec2), glm::value_ptr(vertex.tex_coords));
        uint32_t block_id = static_cast<uint32_t>(vertex.block_id);
        glBufferSubData(GL_ARRAY_BUFFER, offset + sizeof(glm::vec3) + sizeof(glm::vec2), sizeof(BlockType), &block_id);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    size = block_mesh.vertices.size();
}
MeshBuffer::MeshBuffer(uint32_t size) {
    ASSERT_ON_GL_ERROR();

    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindVertexArray(vao);

    glBufferData(GL_ARRAY_BUFFER, size * BlockVertex::gl_size, nullptr, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, BlockVertex::gl_size, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, BlockVertex::gl_size, (void*)(sizeof(glm::vec3)));
    glEnableVertexAttribArray(1);

    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, BlockVertex::gl_size, (void*)(sizeof(glm::vec2) + sizeof(glm::vec3)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    ASSERT_ON_GL_ERROR();

    size = 0;
    capacity = size;
}
MeshBuffer::~MeshBuffer() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}