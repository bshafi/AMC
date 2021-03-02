#pragma once

#include "camera.hpp"
#include "chunk.hpp"
#include "player.hpp"
#include "inventory.hpp"

struct World {
    std::vector<Chunk> chunks;

    Player player;
    Inventory inventory;

    Texture orientation_texture;

    Texture blocks_texture;
    Shader shader;

    glm::vec3 try_move_to(const glm::vec3 &pos, const glm::vec3 &delta_pos, const AABB &aabb) const;
    bool intersects_block(const glm::vec3 &pos, const AABB &aabb) const;

    unsigned int cube_vertices_VBO;
    unsigned int VAO;
    unsigned int block_ids_VBO;


    std::string save_name;
    
    unsigned int globals_3d_ubo;

    static constexpr uint32_t WORLD_VERSION = 1;


    static const uint32_t DEFAULT_SEED = 0x33333333;

    void generate(uint32_t seed = DEFAULT_SEED) noexcept;
    void load(const std::string &path);
    void save(const std::string &path) const;

    World();
    ~World();

    World& operator=(const World&) = delete;

    void handle_events(const std::vector<SDL_Event> &events);
    void draw();
};