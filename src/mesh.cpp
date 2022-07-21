#include "gl_helper.hpp"

#include "hello_cube.hpp"
#include "chunk.hpp"
#include "mesh.hpp"

struct BlockMeshIterator {
    std::array<BlockVertex, 36> vertices;
    glm::ivec3 pos;
    BlockMeshIterator(const Chunk &);
    bool next();
    const Chunk &chunk;
};

BlockMeshIterator::BlockMeshIterator(const Chunk &chunk)
    : pos(glm::ivec3()), chunk(chunk) {

}
bool BlockMeshIterator::next() {
    const glm::ivec3 deltas[] = {
        glm::ivec3(-1, 0, 0), 
        glm::ivec3(1, 0, 0), 
        glm::ivec3(0, -1, 0), 
        glm::ivec3(0, 1, 0), 
        glm::ivec3(0, 0, -1), 
        glm::ivec3(0, 0, 1)
    };

    bool has_put_block = false;
    while (Chunk::is_within_chunk_bounds(pos) && !has_put_block) {
        BlockType block_type = chunk.GetBlock(pos);
        if (block_type != BlockType::Air) {
            bool is_surrounded = true;
            for (uint32_t i = 0; i < 6; ++i) {
                auto neighbor_pos = pos + deltas[i];
                if (!Chunk::is_within_chunk_bounds(neighbor_pos)) {
                    is_surrounded = false;
                    break;
                }
                is_surrounded &= chunk.GetBlock(neighbor_pos) != BlockType::Air;
            }
            if (!is_surrounded) {
                const glm::vec3 center = glm::vec3(pos) + glm::vec3(0.5f, 0.5f, 0.5f);

                for (uint32_t i = 0; i < 36; ++i) {
                    const uint32_t offset = i * 5;
                    BlockVertex vertex = {
                        center + glm::vec3(cube_vertices[offset + 0], cube_vertices[offset + 1], cube_vertices[offset + 2]),
                        glm::vec2(cube_vertices[offset + 3], cube_vertices[offset + 4]),
                        block_type,
                        pos
                    };
                    this->vertices[i] = vertex;
                }
                has_put_block = true;
            }
        }

        Chunk::loop_through(pos);
    }

    return has_put_block;
}

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
                block_type,
                pos
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

uint32_t calc_verts(const Chunk &chunk) {
    BlockMeshIterator bmi(chunk);

    uint32_t i = 0;
    while (bmi.next()) {
        ++i;
    }

    return i * 36;
}

MeshBuffer::MeshBuffer(const Chunk &chunk)
    :  MeshBuffer(calc_verts(chunk)) {
    rebuild(chunk);
}

void MeshBuffer::rebuild(const Chunk &chunk) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    {
        auto verts = calc_verts(chunk);
        if (verts > capacity) {
            glBufferData(GL_ARRAY_BUFFER, verts * BlockVertex::gl_size, nullptr,  GL_DYNAMIC_DRAW);
        }
    }

    BlockMeshIterator bmi(chunk);

    uint8_t buffer[BlockVertex::gl_size * 256];
    uint32_t total_offset = 0;
    uint32_t offset = 0;
    uint32_t total_vertices = 0;

    while (bmi.next()) {
        for (uint32_t j = 0; j < 36; ++j) {
            auto vertex = bmi.vertices[j];
            memcpy(buffer + (offset), glm::value_ptr(vertex.position), sizeof(glm::vec3));
            memcpy(buffer + (offset + sizeof(glm::vec3)), glm::value_ptr(vertex.tex_coords), sizeof(glm::vec2));
            uint32_t block_id = static_cast<uint32_t>(vertex.block_id);
            memcpy(buffer + (offset + sizeof(glm::vec3) + sizeof(glm::vec2)), &block_id, sizeof(BlockType));
            memcpy(buffer + (offset + sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(BlockType)), glm::value_ptr(vertex.original_location), sizeof(glm::ivec3));
            ++total_vertices;

            offset += BlockVertex::gl_size;

            if (offset + BlockVertex::gl_size >= sizeof(buffer)) {
                glBufferSubData(GL_ARRAY_BUFFER, total_offset, offset, buffer);
                ASSERT_ON_GL_ERROR();
                total_offset += offset;
                offset = 0;
            }

        }
    }
    if (offset > 0) {
        glBufferSubData(GL_ARRAY_BUFFER, total_offset, offset, buffer);
        ASSERT_ON_GL_ERROR();
        total_offset += offset;
        offset = 0;
    }
    ASSERT_ON_GL_ERROR();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    size = total_vertices;
}

MeshBuffer::MeshBuffer(uint32_t size) {
    ASSERT_ON_GL_ERROR();

    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindVertexArray(vao);

    glBufferData(GL_ARRAY_BUFFER, size * BlockVertex::gl_size, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, BlockVertex::gl_size, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, BlockVertex::gl_size, (void*)(sizeof(glm::vec3)));
    glEnableVertexAttribArray(1);

    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, BlockVertex::gl_size, (void*)(sizeof(glm::vec2) + sizeof(glm::vec3)));
    glEnableVertexAttribArray(2);
    
    glVertexAttribIPointer(3, 3, GL_INT, BlockVertex::gl_size, (void*)(sizeof(glm::vec2) + sizeof(glm::vec3) + sizeof(BlockType)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    ASSERT_ON_GL_ERROR();

    this->size = 0;
    capacity = size;
}
MeshBuffer::~MeshBuffer() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}