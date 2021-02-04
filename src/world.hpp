#pragma once

#include "camera.hpp"
#include "chunk.hpp"

struct World {
    std::vector<Chunk> chunks;

    Texture orientation_texture;
    Texture blocks_texture;
    Shader shader;

    unsigned int cube_vertices_VBO;
    unsigned int VAO;
    unsigned int block_ids_VBO;

    Camera camera;
    std::string save_name;
    
    unsigned int globals_3d_ubo;

    static constexpr uint32_t WORLD_VERSION = 1;


    void generate() noexcept;
    void load(const std::string &path);
    void save(const std::string &path) const;

    World();
    ~World();

    World& operator=(const World&) = delete;

    void handle_events(const std::vector<SDL_Event> &events);
    void draw();
};