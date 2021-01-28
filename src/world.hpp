#pragma once

#include "camera.hpp"
#include "chunk.hpp"

struct World {
    Chunk chunk;
    Camera camera;
    unsigned int globals_3d_ubo;

    World();
    ~World();

    World(const World &) = delete;
    World(World &&) = delete;

    void handle_events(const std::vector<SDL_Event> &events);
    void draw();
    void load_world(const std::string &file_path);
    void save_world(const std::string &file_path);
};