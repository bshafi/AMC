#pragma once

#include "../src/world.hpp"

#include "gtest/gtest.h"

#define TEST_FILE_PATH "tests/test_save_file.hex"

SaveFile open_save_file() {
    {
        std::ofstream file(TEST_FILE_PATH, std::ios_base::trunc | std::ios_base::binary);

    }
    return SaveFile(TEST_FILE_PATH);
}

TEST(SaveFile, InitializeSaveFile) {
    {
        SaveFile file = open_save_file();
    }

    std::ifstream file(TEST_FILE_PATH, std::ios_base::binary);
    file.seekg(16);
    for (size_t i = 0; i < 16; ++i) {
        ASSERT_EQ(read_binary<int32_t>(file), INT_MIN);
        ASSERT_EQ(read_binary<int32_t>(file), INT_MIN);
    }
}

TEST(SaveFile, Test0) {
    SaveFile file = open_save_file();

    Chunk test_chunk;

    test_chunk.chunk_pos = glm::ivec2(
        rand() % 101 - 50,
        rand() % 101 - 50
    );
    for(auto pos = glm::ivec3(); Chunk::is_within_chunk_bounds(pos); Chunk::loop_through(pos)) {
        BlockType block = static_cast<BlockType>(static_cast<uint32_t>(rand() % 8));
        test_chunk.GetBlock(pos) = block;
    }

    file.write_chunk(test_chunk);

    Chunk read_chunk;
    file.read_chunk(test_chunk.chunk_pos, read_chunk);

    ASSERT_EQ(read_chunk.chunk_pos, test_chunk.chunk_pos);

    for(auto pos = glm::ivec3(); Chunk::is_within_chunk_bounds(pos); Chunk::loop_through(pos)) {
        ASSERT_EQ(read_chunk.GetBlock(pos), test_chunk.GetBlock(pos));
    }
}
TEST(SaveFile, Test1) {
    std::unordered_map<glm::ivec2, Chunk> rand_chunks;
    for (size_t i = 0; i < 20; ++i) {
        Chunk chunk;
        chunk.chunk_pos = glm::ivec2(
            rand() % 101 - 50,
            rand() % 101 - 50
        );
        for(auto pos = glm::ivec3(); Chunk::is_within_chunk_bounds(pos); Chunk::loop_through(pos)) {
            BlockType block = static_cast<BlockType>(static_cast<uint32_t>(rand() % 8));
            chunk.GetBlock(pos) = block;
        }
        rand_chunks[chunk.chunk_pos] = chunk;
    }

    SaveFile file = open_save_file();
    for (const auto &[pos, chunk] : rand_chunks) {
        file.write_chunk(chunk);
    }

    bool chunk_was_not_found = false;

    std::unordered_map<glm::ivec2, Chunk> read_chunks;
    for (const auto &[pos, h_chunk] : rand_chunks) {
        Chunk chunk;
        if (file.read_chunk(pos, chunk)) {
            read_chunks[chunk.chunk_pos] = chunk;
        }
    }

    for (const auto &[pos, rand_chunk] : rand_chunks) {
        auto loc = read_chunks.find(pos);
        if (loc != read_chunks.end()) {
            for(auto pos = glm::ivec3(); Chunk::is_within_chunk_bounds(pos); Chunk::loop_through(pos)) {
                ASSERT_EQ(loc->second.GetBlock(pos), rand_chunk.GetBlock(pos));
            }
        } else {
            std::cout << "Chunk was not found:" << pos.x << ", " << pos.y << std::endl;
            chunk_was_not_found = true;
        }
    }

    ASSERT_FALSE(chunk_was_not_found);
}

TEST(SaveFile, Test2) {
    std::unordered_map<glm::ivec2, Chunk> rand_chunks;
    {
        for (size_t i = 0; i < 20; ++i) {
            Chunk chunk;
            chunk.chunk_pos = glm::ivec2(
                rand() % 101 - 50,
                rand() % 101 - 50
            );
            for(auto pos = glm::ivec3(); Chunk::is_within_chunk_bounds(pos); Chunk::loop_through(pos)) {
                BlockType block = static_cast<BlockType>(static_cast<uint32_t>(rand() % 8));
                chunk.GetBlock(pos) = block;
            }
            rand_chunks[chunk.chunk_pos] = chunk;
        }

        SaveFile file = open_save_file();
        for (const auto &[pos, chunk] : rand_chunks) {
            file.write_chunk(chunk);
        }
    }
    SaveFile file{TEST_FILE_PATH};
    
    bool chunk_was_not_found = false;

    std::unordered_map<glm::ivec2, Chunk> read_chunks;
    for (const auto &[pos, h_chunk] : rand_chunks) {
        Chunk chunk;
        if (file.read_chunk(pos, chunk)) {
            read_chunks[chunk.chunk_pos] = chunk;
        }
    }

    for (const auto &[pos, rand_chunk] : rand_chunks) {
        auto loc = read_chunks.find(pos);
        if (loc != read_chunks.end()) {
            for(auto pos = glm::ivec3(); Chunk::is_within_chunk_bounds(pos); Chunk::loop_through(pos)) {
                ASSERT_EQ(loc->second.GetBlock(pos), rand_chunk.GetBlock(pos));
            }
        } else {
            std::cout << "Chunk was not found:" << pos.x << ", " << pos.y << std::endl;
            chunk_was_not_found = true;
        }
    }

    ASSERT_FALSE(chunk_was_not_found);
}
