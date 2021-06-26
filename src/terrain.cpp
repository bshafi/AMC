#pragma once

#include "terrain.hpp"
#include "world.hpp"

Terrain::Terrain(std::unique_ptr<ChunkLoader> &&save_file)
    : save_file(std::move(save_file)), blocks_texture("resources/blocks.png"), shader("shaders/block.vert", "shaders/chunk.frag") {
    
}
BlockType& Terrain::block(const BlockHit &hit) {
    return this->block(hit.chunk_pos, hit.block_pos);
}
BlockType& Terrain::block(const glm::ivec2 &chunk_pos, const glm::ivec3 &block_pos) {
    return chunks[chunk_pos].GetBlock(block_pos);
}
BlockType& Terrain::block(const glm::ivec3 &world_pos) {
    const glm::ivec2 chunk_pos = glm::ivec2(world_pos.x, world_pos.z) / glm::ivec2(Chunk::CHUNK_WIDTH, Chunk::CHUNK_WIDTH);
    const glm::ivec3 block_pos = world_pos - glm::ivec3(chunk_pos.x * Chunk::CHUNK_WIDTH, 0, chunk_pos.y * Chunk::CHUNK_WIDTH);
    return this->block(chunk_pos, block_pos);
}
bool Terrain::intersects_bounding_box(const BoundingBox &bounding_box) const {
    glm::vec3 bbl_relative_to_chunk = bounding_box.back_bottom_left() / static_cast<float>(Chunk::CHUNK_WIDTH);
    glm::vec3 tfr_relative_to_chunk = bounding_box.top_front_right() / static_cast<float>(Chunk::CHUNK_WIDTH);
    glm::ivec3 bbl_chunk = glm::ivec3(glm::floor(bbl_relative_to_chunk));
    glm::ivec3 tfr_chunk = glm::ivec3(glm::floor(tfr_relative_to_chunk));

    for (int z = bbl_chunk.z; z <= tfr_chunk.z; ++z)
        for (int x = bbl_chunk.x; x <= tfr_chunk.x; ++x) {
            glm::ivec2 chunk_pos = glm::ivec2(x, z);
            auto chunk_loc = chunks.find(chunk_pos);
            if (chunk_loc == chunks.end()) {
                continue;
            }
            if (chunk_loc->second.intersects(bounding_box.pos, bounding_box.aabb)) {
                return true;
            }
        }
    return false;
}
std::optional<BlockHit> Terrain::block_ray_cast(const Ray &ray, const float length) const {
    float min_t = std::numeric_limits<float>::infinity();
    std::optional<BlockHit> block = std::nullopt;
    glm::ivec3 location;
    for (auto &[chunk_pos, chunk] : chunks) {
        std::optional<float> hit = ray.cast(chunk, 100.0f);
        if (!hit.has_value()) {
            continue;
        }
        glm::vec3 hit_pos = (*hit) * ray.direction + ray.endpoint;
        location = get_hit_block(chunk, ray.direction, hit_pos);
        assert(chunk_contains(chunk, location));
        if ((*hit) < min_t) {
            min_t = *hit;
            block = BlockHit{ chunk_pos, location, get_hit_face(hit_pos, location) };
        }
    }
    return block;
}
void Terrain::bind_UBO(const std::string &name, uint32_t loc) {
    this->shader.bind_UBO("globals_3d", loc);
}

template <>
bool intersects(const Terrain &terrain, const BoundingBox &bounding_box) {
    return terrain.intersects_bounding_box(bounding_box);
}
std::optional<float> Terrain::ray_cast(const Ray &ray, const float length) const {
    auto hit = block_ray_cast(ray, length);
    if (hit.has_value()) {
        glm::ivec3 world_pos = hit->block_pos + glm::ivec3(hit->chunk_pos.x * Chunk::CHUNK_WIDTH, 0, hit->chunk_pos.y * Chunk::CHUNK_WIDTH);
        glm::vec3 delta_ts = (glm::vec3(world_pos) - ray.endpoint) / ray.direction;
        float t = std::min({ delta_ts.x, delta_ts.y, delta_ts.z }, min_nonegative);
        return t;
    } else {
        return std::nullopt;
    }
}
template <>
std::optional<float> Ray::cast(const Terrain &terrain, const float length) const {
    return terrain.ray_cast(*this, length);
}


SaveFile::SaveFile(const std::string &file_path) {
    this->path = file_path;
    this->file.open(file_path, std::ios_base::binary | std::ios_base::in | std::ios_base::out);
    
    try {
        std::fstream &file = this->file;
        std::string &path = this->path;
        file.exceptions(~std::ios_base::goodbit);

        const auto pad16 = [&file]() {
            while (file.tellg() % 16 != 0) {
                read_binary<uint8_t>(file);
            }
        };

        uint64_t i = 0;
        while (!file.eof()) {
            glm::ivec2 chunk_pos(
                read_binary<int32_t>(file),
                read_binary<int32_t>(file)
            );
            pad16();
            file.seekg(std::ios_base::beg, i * Chunk::BLOCKS_IN_CHUNK * sizeof(BlockType));
            ++i;
        }

        file.close();
    } catch(std::ios_base::failure &error) {
        std::cout << "File could not open: " << error.what() << std::endl;
        std::cout << "Generating new world instead" << std::endl;

        return World::generate();
    }
}
SaveFile::~SaveFile() {

}

Chunk SaveFile::read_chunk(const glm::ivec2 &chunk_pos) {

}
void SaveFile::write_chunk(const Chunk &chunk) {

}