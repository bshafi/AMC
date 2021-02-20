#include <iostream>

#include "gl_helper.hpp"
#include "hello_cube.hpp"
#include "chunk.hpp"

Chunk::Chunk() {
}

Chunk::~Chunk() {

}

void Chunk::loop_through(glm::ivec3 &pos) {
    assert(MIN_X <= pos.x && pos.x <= MAX_X);
    assert(MIN_Y <= pos.y && pos.y <= MAX_Y);
    assert(MIN_Z <= pos.z && pos.z <= MAX_Z);
    ++pos.x;
    if (pos.x > MAX_X) {
        pos.x = 0;
        ++pos.z;
        if (pos.z > MAX_Z) {
            pos.z = 0;
            ++pos.y;
            if (pos.y > MAX_Y) {
                // Do nothing
            }
        }
    }

    // The loop may be out of bounds after the last call of this
    // assert(0 <= pos.x && pos.x < 16);
    // assert(0 <= pos.y && pos.y < 256);
    // assert(0 <= pos.z && pos.z < 16);
}
bool Chunk::is_within_chunk_bounds(const glm::ivec3 &pos) {
    return (MIN_X <= pos.x && pos.x <= MAX_X) &&
    (MIN_Y <= pos.y && pos.y <= MAX_Y) &&
    (MIN_Z <= pos.z && pos.z <= MAX_Z);
}

// the result of the modulo operator can be undefined on negative numbers this operations preserves the mathematical definition of modulo
// -16 | ... | -2 | -1 | 0 | 1 | 2
//+  0 | ... | 14 | 15 | 0 | 1 | 2
uint32_t mod16(int32_t x) {
    if (x < 0) {
        return (16 - ((-x) % 16)) % 16;
    } else {
        return x % 16;
    }
}

// pos.y must be between 0 and 255
uint32_t Chunk::world_pos_to_index(glm::ivec3 pos) {
    assert(MIN_Y <= pos.y && pos.y <= MAX_Y);

    uint32_t x = mod16(pos.x);
    uint32_t y = pos.y;
    uint32_t z = mod16(pos.z);
    return (x + 16 * (z + y * 16));
}
Chunk::BlockIDType Chunk::GetBlock(glm::ivec3 pos) const {
    assert(MIN_X <= pos.x && pos.x <= MAX_X);
    assert(MIN_Y <= pos.y && pos.y <= MAX_Y);
    assert(MIN_Z <= pos.z && pos.z <= MAX_Z);

    return blocks[world_pos_to_index(pos)];
}
void Chunk::SetBlock(glm::ivec3 pos, Chunk::BlockIDType id) {
    assert(MIN_X <= pos.x && pos.x <= MAX_X);
    assert(MIN_Y <= pos.y && pos.y <= MAX_Y);
    assert(MIN_Z <= pos.z && pos.z <= MAX_Z);

    blocks[world_pos_to_index(pos)] = id;
}

bool Chunk::intersects(glm::vec3 pos, AABB aabb) const {
    glm::vec3 chunk_pos_fvec3 = { this->chunk_pos.x * CHUNK_WIDTH, 0, this->chunk_pos.y * CHUNK_WIDTH };
    AABB chunk_aabb = { CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_WIDTH };

    // Check if the thing intersects the entire chunk in the first place
    if (!AABBIntersection(chunk_pos_fvec3, chunk_aabb, pos, aabb)) {
        return false;
    }

    glm::ivec3 bottom_left_back = floor(pos - chunk_pos_fvec3);
    glm::ivec3 top_right_front = floor(pos + glm::vec3(aabb.width, aabb.length, aabb.height) - chunk_pos_fvec3);

    assert(bottom_left_back.x <= top_right_front.x);
    assert(bottom_left_back.y <= top_right_front.y);
    assert(bottom_left_back.z <= top_right_front.z);

    for (int i = bottom_left_back.x; i <= top_right_front.x; ++i)
        for (int j = bottom_left_back.y; j <= top_right_front.y; ++j)
            for (int k = bottom_left_back.z; k <= top_right_front.z; ++k) {
                const glm::ivec3 checking_pos = glm::ivec3(i, j, k);
                const glm::vec3 block_pos_world_coords = glm::vec3(checking_pos) + chunk_pos_fvec3;
            
                // The object that is being intersected against may have some part of itself
                // outside the chunk boundaries
                if (is_within_chunk_bounds(checking_pos)) {
                    if (this->GetBlock(checking_pos) != BlockType::Air && AABBIntersection(block_pos_world_coords, AABB{ 1, 1, 1}, pos, aabb)) {
                        return true;
                    }
                }
            }

    return false;
}