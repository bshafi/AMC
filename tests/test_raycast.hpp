#pragma once

#include "../src/chunk.hpp"
#include "../src/standard.hpp"

#include "gtest/gtest.h"

TEST(TestRayCast, Test) {
    Chunk chunk;
    for (int i = 0; i < 10; ++i) {
        chunk.GetBlock(glm::ivec3(i, 0, 0)) = BlockType::Stone;
    }
    Ray ray = { glm::vec3(0, -10, 0) , glm::normalize(glm::vec3(0, 1, 0)) };
    auto hit = ray.cast(chunk, 10);
    if (hit.has_value()) {
        BlockType block_type = chunk.GetBlock(glm::ivec3((*hit) * ray.direction + ray.endpoint));
        if (block_type != BlockType::Stone) {
            std::cout << *hit << std::endl;
            const glm::vec3 thing = (*hit) * ray.direction + ray.endpoint;
            std::cout << thing.x << ", " << thing.y << ", " << thing.z << std::endl;
            std::cout << static_cast<uint32_t>(block_type);
        }
    } else {
        std::cout << "nullopt" << std::endl;
    }
}

glm::ivec3 to_loc(uint32_t pos);

TEST(TestBlockIndexing, Test) {
    Chunk chunk;
    uint32_t error_count = 0;
    uintptr_t start = reinterpret_cast<uintptr_t>(&chunk.GetBlock(glm::ivec3(0, 0, 0)));
    for (auto pos = glm::ivec3(); Chunk::is_within_chunk_bounds(pos); Chunk::loop_through(pos)) {
        uintptr_t end = reinterpret_cast<uintptr_t>(&chunk.GetBlock(pos));
        
        uintptr_t diff;
        if (end > start) {
            diff = end - start;
        } else {
            diff = start - end;
        }

        assert(diff % sizeof(glm::i32) == 0);
        diff /= sizeof(glm::i32);
        glm::ivec3 h_pos = to_loc(static_cast<uint32_t>(diff));
        if (pos != h_pos) {
            std::cout << "diff:  " << diff << "\n";
            std::cout << "pos:   " << pos.x << ", " << pos.y << ", " << pos.z << "\n";
            std::cout << "h_pos: " << h_pos.x << ", " << h_pos.y << ", " << h_pos.z << "\n";
            ++error_count;
        }
        if (error_count > 10) {
            break;
        }
    }
    std::cout << std::flush;
}
glm::ivec3 to_loc(uint32_t gl_InstanceID) {
    return glm::ivec3(
        static_cast<glm::i32>(gl_InstanceID / (16 * 256)),
        static_cast<glm::i32>((gl_InstanceID % (16 * 256)) / 16),
        static_cast<glm::i32>((gl_InstanceID % (16 * 256)) % 16)
    );
}
