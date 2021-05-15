#pragma once

#include "camera.hpp"
#include "chunk.hpp"
#include "player.hpp"
#include "inventory.hpp"

class Renderer;


struct World {
    std::vector<Chunk> chunks;
    std::vector<uint32_t> vertex_buffer_objects;
    std::vector<uint32_t> vertex_buffer_objects_size;

    Player player;
    Inventory inventory;

    Texture orientation_texture;

    Texture blocks_texture;
    Shader shader;

    glm::vec3 try_move_to(const glm::vec3 &pos, const glm::vec3 &delta_pos, const AABB &aabb) const;
    bool intersects_block(const glm::vec3 &pos, const AABB &aabb) const;

    uint32_t VAO;

    std::string save_name;
    
    uint32_t globals_3d_ubo;

    static constexpr uint32_t WORLD_VERSION = 1;
    static const uint32_t DEFAULT_SEED = 0x33333333;
    static void gen_chunk_blocks(const Chunk &chunk, std::vector<glm::uvec4> &data);

    void generate(uint32_t seed = DEFAULT_SEED);
    void load(const std::string &path);
    void save(const std::string &path) const;

    void destroy_chunks();

    World();
    ~World();

    World& operator=(const World&) = delete;

    void handle_events(const std::vector<SDL_Event> &events);
    void draw();
};