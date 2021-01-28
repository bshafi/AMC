#include <fstream>

#include "gl_helper.hpp"

#include "world.hpp"

World::World() {
    ASSERT_ON_GL_ERROR();

    glGenBuffers(1, &globals_3d_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, globals_3d_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, nullptr, GL_STATIC_DRAW);
    glm::mat4 view = camera.view_matrix();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
    glm::mat4 projection = glm::perspective(static_cast<float>(M_PI / 4), 640.f / 480.f, 0.1f, 100.f);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));

    chunk.shader.bind_UBO("globals_3d", 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, globals_3d_ubo);

    ASSERT_ON_GL_ERROR();
}
World::~World() {
    ASSERT_ON_GL_ERROR();

    glDeleteBuffers(1, &globals_3d_ubo);

    ASSERT_ON_GL_ERROR();
}

void World::handle_events(const std::vector<SDL_Event> &events) {
    ASSERT_ON_GL_ERROR();

    for (const auto event : events) {
        switch (event.type) {
        case SDL_WINDOWEVENT: {
            switch (event.window.type) {
            case SDL_WINDOWEVENT_RESIZED:
                glViewport(0, 0, event.window.data1, event.window.data2);
                glBindBuffer(GL_UNIFORM_BUFFER, globals_3d_ubo);
                glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, nullptr, GL_DYNAMIC_DRAW);
                glm::mat4 projection = glm::perspective(static_cast<float>(M_PI / 4), static_cast<float>(event.window.data1) / event.window.data2, 0.1f, 100.f);
                glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
            }
        }
            break;
        default:
            break;
        }
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
        if (event.type == SDL_MOUSEMOTION && SDL_GetRelativeMouseMode() == SDL_TRUE) {
            camera.rotate_right(M_PI * event.motion.xrel / 1000.0f);
            camera.rotate_upwards(-M_PI * event.motion.yrel / 1000.0f);
        }

        ASSERT_ON_GL_ERROR();
    }

    const auto keypresses = SDL_GetKeyboardState(NULL);
    if (keypresses[SDL_SCANCODE_A]) {
        camera.pos(camera.pos() - camera.right() * 0.1f);
    }
    if (keypresses[SDL_SCANCODE_D]) {
        camera.pos(camera.pos() + camera.right() * 0.1f);
    }
    if (keypresses[SDL_SCANCODE_S])  {
        camera.pos(camera.pos() - camera.forward() * 0.1f);
    }
    if (keypresses[SDL_SCANCODE_W])  {
        camera.pos(camera.pos() + camera.forward() * 0.1f);
    }
}
void World::draw() {
    ASSERT_ON_GL_ERROR();

    
     
    glBindBuffer(GL_UNIFORM_BUFFER, globals_3d_ubo);
    glm::mat4 view = camera.view_matrix();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    ASSERT_ON_GL_ERROR();

    chunk.draw();

    ASSERT_ON_GL_ERROR();
}
void World::load_world(const std::string &file_path) {
    std::ifstream world_file(file_path);
    
    assert(world_file.is_open() && world_file.good());

    int x, y;
    assert(world_file >> x && world_file >> y);

    std::array<Chunk::BlockIDType, Chunk::BLOCKS_IN_CHUNK> blocks;

    Chunk::BlockIDType block_id;
    for (unsigned i = 0; i < Chunk::BLOCKS_IN_CHUNK; ++i) {
        assert(world_file >> block_id);
        blocks[i] = block_id;
    }

    this->chunk.load_blocks(blocks);
    

    // TODO: Implement Chunk position
}
void World::save_world(const std::string &file_path) {

    std::ofstream world_file(file_path);

    assert(world_file.good() && world_file.is_open());

    // FIXME: Implement Chunk position
    world_file << 0 << " " << 0 << "\n";
    
    std::array<Chunk::BlockIDType, Chunk::BLOCKS_IN_CHUNK> blocks;
    this->chunk.save_blocks(blocks);

    for (int i = 0; i < Chunk::BLOCKS_IN_CHUNK; ++i) {
        if (i % 16 == 0) {
            world_file << "\n";
        }
        if (i % 256 == 0) {
            world_file << "\n";
        }

        world_file << blocks[i] << " ";
    }
    world_file << "\n" << std::endl;

    world_file.close();
}