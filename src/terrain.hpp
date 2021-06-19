#pragma once

#include <fstream>
#include <unordered_map>

#include "standard.hpp"
#include "chunk.hpp"

class Terrain {
public:
    virtual Chunk* GetChunk(glm::ivec2 pos) = 0;
    virtual const Chunk* GetChunk(glm::ivec2 pos) const = 0;

    virtual void AllocateChunk(const glm::ivec2 &pos) = 0;
    virtual void DeallocateChunk(const glm::ivec2 &pos) = 0;
private:
};

class SaveTerrain : public Terrain {
public:
    SaveTerrain(const std::string &path);

    virtual Chunk* GetChunk(glm::ivec2 pos) override;
    virtual const Chunk* GetChunk(glm::ivec2 pos) const override;

    virtual void AllocateChunk(const glm::ivec2 &pos) override;
    virtual void DeallocateChunk(const glm::ivec2 &pos) override;
private:
    void LoadChunk(const glm::ivec2 &pos);

    std::fstream save_file;
    std::unordered_map<glm::ivec2, std::streampos> chunk_positions;
    std::unordered_map<glm::ivec2, Chunk> chunks;
};

class ServerTerrain : public Terrain {
public:
    virtual Chunk* GetChunk(glm::ivec2 pos) override;
    virtual const Chunk* GetChunk(glm::ivec2 pos) const override;

    virtual void AllocateChunk(const glm::ivec2 &pos) override;
    virtual void DeallocateChunk(const glm::ivec2 &pos) override;
private:
};