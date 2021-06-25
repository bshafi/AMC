#include <iostream>
#include <fstream>

#include "gl_helper.hpp"
#include "hello_cube.hpp"
#include "chunk.hpp"

#include "PerlinNoise.hpp"

frect BlockRect(const BlockType &type) {
    return frect{
        0,
        static_cast<float>(static_cast<uint32_t>(type) - 1) * 8,
        8,
        8
    };
}


Chunk::Chunk()
    : chunk_pos{ 0, 0 } {
    
    for (auto pos = glm::ivec3(); Chunk::is_within_chunk_bounds(pos); Chunk::loop_through(pos)) {
        GetBlock(pos) = BlockType::Air;
    }
}

Chunk::~Chunk() {

}
/*
static Chunk generate(const Biome current, const glm::ivec2 chunk_pos) {
    const float frequency = 1.f;
    const double fx = (Chunk::CHUNK_WIDTH * 5) / frequency;
    const double fy = (Chunk::CHUNK_WIDTH * 5) / frequency;
    const uint32_t octaves = 4;
    siv::PerlinNoise noise(0x33333333);

    Chunk chunk;

    switch (current) {
    case Biome::Unkown: assert(false); break;
    case Biome::Plains: {
        for (auto pos = glm::ivec3(); Chunk::is_within_chunk_bounds(pos); Chunk::loop_through(pos)) {
            BlockType block_id = BlockType::Air;
            assert(0 <= pos.x && pos.x < Chunk::CHUNK_WIDTH);
            assert(0 <= pos.z && pos.z < Chunk::CHUNK_WIDTH);

            double r_noise = noise.accumulatedOctaveNoise2D_0_1((pos.x + (chunk_pos.x * Chunk::CHUNK_WIDTH))  / fx, (pos.z + (chunk_pos.y * Chunk::CHUNK_WIDTH)) / fy, octaves);
            const uint8_t height = static_cast<uint8_t>(std::clamp(r_noise * 128, 0.0, 255.0));
            if (pos.y <= height) {
                if (pos.y == height) {
                    block_id = BlockType::Grass;
                } else if (pos.y + 4 > height) {
                    block_id = BlockType::Dirt;
                } else {
                    block_id = BlockType::Stone;
                }
            }
            chunk.GetBlock(pos) = block_id;
        }
    }
        break;
    default:
        assert(false);
    }

    return chunk;
}
*/

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
        return (16 - ((-x) % 16));
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

const BlockType &Chunk::GetBlock(glm::ivec3 pos) const {
    assert(MIN_X <= pos.x && pos.x <= MAX_X);
    assert(MIN_Y <= pos.y && pos.y <= MAX_Y);
    assert(MIN_Z <= pos.z && pos.z <= MAX_Z);

    //return blocks[world_pos_to_index(pos)];
    return blocks[pos.x][pos.y][pos.z];
}
BlockType &Chunk::GetBlock(glm::ivec3 pos) {
    assert(MIN_X <= pos.x && pos.x <= MAX_X);
    assert(MIN_Y <= pos.y && pos.y <= MAX_Y);
    assert(MIN_Z <= pos.z && pos.z <= MAX_Z);

    //return blocks[world_pos_to_index(pos)];
    return blocks[pos.x][pos.y][pos.z];
}
BlockType Chunk::GetBlockFromWorld(glm::ivec3 pos) const {
    return GetBlock(glm::ivec3(glm::vec3(pos) - world_pos()));
}
/*
void Chunk::SetBlock(glm::ivec3 pos, BlockType id) {
    assert(MIN_X <= pos.x && pos.x <= MAX_X);
    assert(MIN_Y <= pos.y && pos.y <= MAX_Y);
    assert(MIN_Z <= pos.z && pos.z <= MAX_Z);

    blocks[pos.x][pos.y][pos.z] = id;
}*/

bool Chunk::intersects(glm::vec3 pos, AABB aabb) const {
    glm::vec3 chunk_pos_world_coords = this->world_pos();
    AABB chunk_aabb = this->bounding_box();

    // Check if the thing intersects the entire chunk in the first place
    if (! ::intersects(BoundingBox{ chunk_pos_world_coords, chunk_aabb }, BoundingBox{ pos, aabb })) {
        return false;
    }

    glm::ivec3 bottom_left_back = glm::ivec3(floor(pos - chunk_pos_world_coords));
    glm::ivec3 top_right_front = glm::ivec3(floor(pos + glm::vec3(aabb.width, aabb.length, aabb.height) - chunk_pos_world_coords));

    assert(bottom_left_back.x <= top_right_front.x);
    assert(bottom_left_back.y <= top_right_front.y);
    assert(bottom_left_back.z <= top_right_front.z);

    for (int i = bottom_left_back.x; i <= top_right_front.x; ++i)
        for (int j = bottom_left_back.y; j <= top_right_front.y; ++j)
            for (int k = bottom_left_back.z; k <= top_right_front.z; ++k) {
                const glm::ivec3 checking_pos = glm::ivec3(i, j, k);
                const glm::vec3 block_pos_world_coords = glm::vec3(checking_pos) + chunk_pos_world_coords;
            
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
AABB Chunk::bounding_box() const {
    return AABB{ CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_WIDTH };
}
glm::vec3 Chunk::world_pos() const {
    return glm::vec3(this->chunk_pos.x * CHUNK_WIDTH, 0, this->chunk_pos.y * CHUNK_WIDTH);
}

// FIXME: Chunk contains should check if pos is right
bool chunk_contains(const Chunk &chunk, const glm::ivec3 &pos) {
    return (Chunk::MIN_X <= pos.x && pos.x <= Chunk::MAX_X) &&
           (Chunk::MIN_Y <= pos.y && pos.y <= Chunk::MAX_Y) &&
           (Chunk::MIN_Z <= pos.z && pos.z <= Chunk::MAX_Z);
}

bool min_nonegative(float a, float b) {
    if (a < 0) {
        a = std::numeric_limits<float>::infinity();
    }
    if (b < 0) {
        b = std::numeric_limits<float>::infinity();
    }
    return a < b;
}



glm::ivec3 get_hit_block(const Chunk &chunk, const glm::vec3 &dir, const glm::vec3 &hit_pos) {
    float t = std::min({
            (roundup(hit_pos.x) - hit_pos.x) / fabs(dir.x),
            (roundup(hit_pos.y) - hit_pos.y) / fabs(dir.y),
            (roundup(hit_pos.z) - hit_pos.z) / fabs(dir.z),
        }, 
        min_nonegative
    );
    const glm::vec3 next_hit = hit_pos + t * dir;
    return glm::ivec3(
        static_cast<int32_t>(floor(std::min(hit_pos.x, next_hit.x))),
        static_cast<int32_t>(floor(std::min(hit_pos.y, next_hit.y))),
        static_cast<int32_t>(floor(std::min(hit_pos.z, next_hit.z)))
    ) - glm::ivec3(chunk.world_pos());
}



bool has_left_chunk(const glm::vec3 &dir, const glm::vec3 &hit_pos, const Chunk &chunk) {
    const glm::vec3 bounding_box = glm::vec3(chunk.bounding_box().width, chunk.bounding_box().height, chunk.bounding_box().length);
    glm::vec3 corners[] = {
        chunk.world_pos() + glm::vec3(0, 0, 0) * bounding_box,
        chunk.world_pos() + glm::vec3(0, 0, 1) * bounding_box,
        chunk.world_pos() + glm::vec3(0, 1, 0) * bounding_box,
        chunk.world_pos() + glm::vec3(0, 1, 1) * bounding_box,
        chunk.world_pos() + glm::vec3(1, 0, 0) * bounding_box,
        chunk.world_pos() + glm::vec3(1, 0, 1) * bounding_box,
        chunk.world_pos() + glm::vec3(1, 1, 0) * bounding_box,
        chunk.world_pos() + glm::vec3(1, 1, 1) * bounding_box,
    };
    for (int i = 0; i < 8; ++i) {
        if (glm::dot(corners[i] - hit_pos, dir) >= 0.0f) {
            return false;
        }
    }
    return true;
}
// TODO: Clean up all this code
template <>
std::optional<float> Ray::cast(const Chunk &chunk, const float length) const {
    const BoundingBox chunk_bounding_box = BoundingBox{ chunk.world_pos(), chunk.bounding_box() };
    std::optional<float> hit = this->cast(chunk_bounding_box, length);

    if (!hit.has_value()) {
        return std::nullopt;
    }

    float t = *hit;

    assert(fabs(glm::length(direction) - 1.0f) <= 0.0001f);
    glm::vec3 hit_pos = t * direction + endpoint;
    glm::ivec3 local_pos = get_hit_block(chunk, direction, hit_pos);


    std::optional<BlockType> block;
    if (chunk_contains(chunk, local_pos)) {
        block = chunk.GetBlock(local_pos);
    }

    while (
        (!block.has_value() || block == BlockType::Air) &&
        t <= length
    ) {
        // how far each element needs to go to hit a face of the next block
        const glm::vec3 distance = roundup(hit_pos) - hit_pos;

        // choose the smallest delta_t to take one of the basis vectors to a face of the next block
        float delta_t = std::min({ distance.x / fabs(direction.x), distance.y / fabs(direction.y), distance.z / fabs(direction.z) }, min_nonegative);
        

        t = t + delta_t;
        hit_pos = t * direction + endpoint;
        local_pos = get_hit_block(chunk, direction, hit_pos);

        
        if (chunk_contains(chunk, local_pos)) {
            block = chunk.GetBlock(local_pos);
        } else {
            block = std::nullopt;
        }

        if (has_left_chunk(direction, hit_pos, chunk)) {
            return std::nullopt;
        }
    }
    if (t > length) {
        return std::nullopt;
    }
    if (block == std::nullopt) {
        return std::nullopt;
    }
    return t;
}

const char *get_face_name(BlockHit::Face face) {
    return face_names[static_cast<uint32_t>(face)];
}