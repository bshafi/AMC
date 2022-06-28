#pragma once

#include <memory>

#include "standard.hpp"
#include "camera.hpp"
#include "chunk.hpp"
#include "player.hpp"
#include "inventory.hpp"
#include "mesh.hpp"

#include "rigidbody.hpp"

class Renderer;

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

    void handle_events(const std::vector<SDL_Event> &events);

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
