#pragma once

#include <fstream>

#include "standard.hpp"
#include "chunk.hpp"
#include "mesh.hpp"

class ChunkLoader;

class Terrain {
public:
    Terrain(std::unique_ptr<ChunkLoader> &&save_file);

    BlockType& block(const BlockHit &hit);
    BlockType& block(const glm::ivec2 &chunk_pos, const glm::ivec3 &block_pos);
    BlockType& block(const glm::ivec3 &world_pos);
    bool intersects_bounding_box(const BoundingBox &bb) const;
    std::optional<float> ray_cast(const Ray &ray, const float) const;
    std::optional<BlockHit> block_ray_cast(const Ray &ray, const float) const;

    void bind_UBO(const std::string &name, uint32_t loc);

    void draw();
private:
    std::unique_ptr<ChunkLoader> save_file;

    Texture blocks_texture;
    Shader shader;

    std::unordered_map<glm::ivec2, Chunk> chunks;
    std::unordered_map<glm::ivec2, MeshBuffer> meshes;
};

class ChunkLoader {
public:
    virtual Chunk read_chunk(const glm::ivec2 &chunk_pos) = 0;
    virtual void write_chunk(const Chunk &chunk) = 0;
    virtual ~ChunkLoader() {

    }
private:
};

class SaveFile : public ChunkLoader {
public:
    SaveFile(const std::string &file_path);
    ~SaveFile();

    virtual Chunk read_chunk(const glm::ivec2 &chunk_pos) override;
    virtual void write_chunk(const Chunk &chunk) override;
private:


    std::string path;

    std::fstream file;
    std::unordered_map<glm::ivec2, uint32_t> chunk_file_pos;
};