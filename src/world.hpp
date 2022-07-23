#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <fstream>
#include <queue>
#include <unordered_set>

#include "standard.hpp"
#include "camera.hpp"
#include "chunk.hpp"
#include "player.hpp"
#include "inventory.hpp"
#include "mesh.hpp"
#include "entity.hpp"

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

using ChunkPtr = std::unique_ptr<Chunk>;

void chunk_loader_loading_fn(std::string path);
class ChunkLoader {
public:
    ChunkLoader(const std::string &path);

    bool ready_to_retrieve() const;

    bool request_chunk(glm::ivec2);
    bool request_unload(ChunkPtr &&);
    void notify_request();
    ChunkPtr retrive_chunk();
private:
    friend void chunk_loader_loading_fn(std::string path);

    static std::thread loading_thread;
    static std::once_flag loading_fn_init_flag;

    static std::mutex fullfilled_lock;
    static std::condition_variable fullfilled_is_empty;
    static std::vector<ChunkPtr> fullfilled_chunks;

    static std::mutex request_lock;
    static std::condition_variable has_made_request;
    static std::queue<glm::ivec2> requested_chunks;
    static std::queue<ChunkPtr> requested_unloads;
};

struct PhysicalWorld {
    std::unordered_set<glm::ivec2> requested_chunks;
    ChunkLoader loader;
    std::unordered_map<glm::ivec2, ChunkPtr> chunks;
    Player player;
    std::optional<BlockHit> selected_block;
    std::vector<Entity> entities;
    int32_t selected_block_damage;
    static constexpr uint32_t BLOCK_DURABILITY = 100;
    static constexpr uint32_t RENDER_DISTANCE = 4;

    Inventory inventory;

    Camera main_camera;

    float frequency = 1.f;
    int32_t octaves = 4;

    static constexpr uint32_t WORLD_VERSION = 1;
    static const uint32_t DEFAULT_SEED = 0x33333333;

    std::string save_name;
    //SaveFile save_file;

    glm::vec3 try_move_to(const glm::vec3 &pos, const glm::vec3 &delta_pos, const AABB &aabb) const;
    bool intersects_block(const glm::vec3 &pos, const AABB &aabb) const;
    std::optional<BlockHit> GetBlockFromRay(const Ray &ray);
    BlockType GetBlock(const BlockHit &block_hit);
    void SetBlock(const BlockHit &block_hit, BlockType type);

    static void generate_chunk(glm::ivec2 p, Chunk &,uint32_t seed = DEFAULT_SEED);
    void load(const std::string &path);

    void handle_events(const std::vector<SDL_Event> &events, float delta_ticks_s);
    void update_chunks();

    PhysicalWorld();
    ~PhysicalWorld();

    PhysicalWorld& operator=(const PhysicalWorld&) = delete;
    PhysicalWorld(const PhysicalWorld&) = delete;
};

struct RenderWorld {
    EditableMCModel zombie_model;
    Texture zombie_texture;
    ModelRenderer model_renderer;
    std::unordered_map<glm::ivec2, MeshBuffer> meshes;
    std::vector<MeshBuffer> empty_meshes;
    Texture orientation_texture;

    Texture blocks_texture;
    Shader shader;
    
    uint32_t globals_3d_ubo;

    void update_chunks(PhysicalWorld &phys);
    void handle_events(const std::vector<SDL_Event> &events);
    void draw(PhysicalWorld &phys);

    RenderWorld(PhysicalWorld &phys);
    ~RenderWorld();

    RenderWorld& operator=(const World &) = delete;
    RenderWorld(const RenderWorld &) = delete;
};
