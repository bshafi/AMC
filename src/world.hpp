#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <fstream>

#include "standard.hpp"
#include "camera.hpp"
#include "chunk.hpp"
#include "player.hpp"
#include "inventory.hpp"
#include "mesh.hpp"

class Renderer;

class SaveFile {
public:
    SaveFile(const std::string &s);
    // returns false if chunk was not found
    bool read_chunk(const glm::ivec2 &pos, Chunk &chunk);

    void write_chunk(const Chunk &chunk);

    ~SaveFile();
    
    static constexpr uint32_t CHUNK_SIZE = Chunk::BLOCKS_IN_CHUNK * sizeof(uint8_t);
    static constexpr uint32_t TABLE_SIZE = 16 * sizeof(int32_t) * 2;
private:

    void initialize_new_file();

    void pad16();
    void skip16();
    void skip_chunk();

    bool read_header();
    void write_header();

    bool read_table();


    std::fstream file;
    std::unordered_map<glm::ivec2, std::streampos> chunk_locations;
    std::streampos last_table;
};

struct PhysicalWorld {
    std::unordered_map<glm::ivec2, Chunk> chunks;
    Player player;
    std::optional<BlockHit> selected_block;
    int32_t selected_block_damage;
    static constexpr uint32_t BLOCK_DURABILITY = 100;
    static constexpr uint32_t RENDER_DISTANCE = 4;

    Inventory inventory;

    float frequency = 1.f;
    int32_t octaves = 4;

    static constexpr uint32_t WORLD_VERSION = 1;
    static const uint32_t DEFAULT_SEED = 0x33333333;

    std::string save_name;

    glm::vec3 try_move_to(const glm::vec3 &pos, const glm::vec3 &delta_pos, const AABB &aabb) const;
    bool intersects_block(const glm::vec3 &pos, const AABB &aabb) const;
    std::optional<BlockHit> GetBlockFromRay(const Ray &ray);
    BlockType GetBlock(const BlockHit &block_hit);
    void SetBlock(const BlockHit &block_hit, BlockType type);

    void generate(uint32_t seed = DEFAULT_SEED) noexcept;
    void load(const std::string &path);
    void save(const std::string &path) const;

    void handle_events(const std::vector<SDL_Event> &events, float delta_ticks_s);

    PhysicalWorld();
    ~PhysicalWorld();

    PhysicalWorld& operator=(const PhysicalWorld&) = delete;
    PhysicalWorld(const PhysicalWorld&) = delete;
};

struct RenderWorld {
    std::unordered_map<glm::ivec2, MeshBuffer> meshes;
    Texture orientation_texture;

    Texture blocks_texture;
    Shader shader;
    
    uint32_t globals_3d_ubo;

    void handle_events(const std::vector<SDL_Event> &events);
    void draw(PhysicalWorld &phys);

    RenderWorld(PhysicalWorld &phys);
    ~RenderWorld();

    RenderWorld& operator=(const World &) = delete;
    RenderWorld(const RenderWorld &) = delete;
};
