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
    const float tex_offsets[] = {
        5.f / 6,
        4.f / 6,
        2.f / 6,
        0.f / 6,
        3.f / 6,
        2.f / 6
    };
    const glm::ivec3 orthos[][2] = {
        { glm::ivec3(0, 1, 0), glm::ivec3(0, 0, 1) },
        { glm::ivec3(0, 1, 0), glm::ivec3(0, 0, 1) },
        { glm::ivec3(1, 0, 0), glm::ivec3(0, 0, 1) },
        { glm::ivec3(1, 0, 0), glm::ivec3(0, 0, 1) },
        { glm::ivec3(1, 0, 0), glm::ivec3(0, 1, 0) },
        { glm::ivec3(1, 0, 0), glm::ivec3(0, 1, 0) },
    };

    for (auto pos = glm::ivec3(); Chunk::is_within_chunk_bounds(pos); Chunk::loop_through(pos)) {
        
        BlockType block_type = chunk.GetBlock(pos);
        if (
            block_type != BlockType::Air &&
            !(
                pos.x + 1 == Chunk::CHUNK_WIDTH ||
                pos.x == 0 ||
                pos.y + 1 == Chunk::CHUNK_HEIGHT ||
                pos.y == 0 ||
                pos.z + 1 == Chunk::CHUNK_WIDTH ||
                pos.z == 0
            )
        ) {
            continue;
        }

        for (uint32_t i = 0; i < 6; ++i) {
            auto neighbor_pos = pos + deltas[i];
            if (!Chunk::is_within_chunk_bounds(neighbor_pos)) {
                continue;
            }
            if (chunk.GetBlock(neighbor_pos) == BlockType::Air) {
                continue;
            }

            glm::vec3 center = glm::vec3(neighbor_pos) + 0.5f * glm::vec3(1, 1, 1) + 0.5f * glm::vec3(deltas[i]);
            glm::vec3 ortho_x = orthos[i][0];
            glm::vec3 ortho_y = orthos[i][1];
            
            glm::vec3 verts[] = {
                center + ortho_x * 0.5f + ortho_y * 0.5f,
                center + ortho_x * -0.5f + ortho_y * 0.5f,
                center + ortho_x * -0.5f + ortho_y * -0.5f,
                center + ortho_x * 0.5f + ortho_y * 0.5f,
                center + ortho_x * -0.5f + ortho_y * -0.5f,
                center + ortho_x * 0.5f + ortho_y * -0.5f,
            };
            glm::vec2 tex_coords[] = {
                glm::vec2(tex_offsets[i] + 1, 1.f),
                glm::vec2(tex_offsets[i] + 0, 1.f),
                glm::vec2(tex_offsets[i] + 0, 0.f),
                glm::vec2(tex_offsets[i] + 1, 1.f),
                glm::vec2(tex_offsets[i] + 0, 0.f),
                glm::vec2(tex_offsets[i] + 1, 0.f),
            };

            for (uint32_t j = 0; j < 6; ++j) {
                mesh.vertices.push_back(BlockVertex{ verts[j], tex_coords[j], block_type });
            }
        }
    }
}