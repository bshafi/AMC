#include <fstream>

#include "PerlinNoise.hpp"

#include "gl_helper.hpp"
#include "hello_cube.hpp"

#include "world.hpp"


void World::destroy_chunks() {
    glDeleteBuffers(vertex_buffer_objects.size(), vertex_buffer_objects.data());
    chunks.clear();
    vertex_buffer_objects.clear();
}

World::World() : 
    orientation_texture{ "resources/hello_cube_orientation.png" },
    blocks_texture{ "resources/blocks.png" }, 
    shader{ "shaders/blocks.vert", "shaders/blocks.frag", "shaders/blocks.geom" } {

    ASSERT_ON_GL_ERROR();

    glGenVertexArrays(1, &VAO);

    ASSERT_ON_GL_ERROR();

    shader.use();
    shader.apply_bindings({
        { "blocks", blocks_texture }
    });

    ASSERT_ON_GL_ERROR();

    glGenBuffers(1, &globals_3d_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, globals_3d_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, nullptr, GL_STATIC_DRAW);
    glm::mat4 view = player.camera.view_matrix();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
    glm::mat4 projection = glm::perspective(static_cast<float>(M_PI / 4), 640.f / 480.f, 0.1f, 100.f);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));

    this->shader.bind_UBO("globals_3d", 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, globals_3d_ubo);

    ASSERT_ON_GL_ERROR();

    player.set_position(glm::vec3(26, 100, 100));
}
World::~World() {
    this->save(save_name);

    ASSERT_ON_GL_ERROR();

    destroy_chunks();
    glDeleteVertexArrays(1, &VAO);

    ASSERT_ON_GL_ERROR();
}

void World::handle_events(const std::vector<SDL_Event> &events) {
    ASSERT_ON_GL_ERROR();

    for (const auto event : events) {
        switch (event.type) {
        case SDL_WINDOWEVENT: {
            switch (event.window.type) {
            case SDL_WINDOWEVENT_RESIZED: {
                glViewport(0, 0, event.window.data1, event.window.data2);
                glBindBuffer(GL_UNIFORM_BUFFER, globals_3d_ubo);
                glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, nullptr, GL_DYNAMIC_DRAW);
                glm::mat4 projection = glm::perspective(static_cast<float>(M_PI / 4), static_cast<float>(event.window.data1) / event.window.data2, 0.1f, 100.f);
                glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
            }
                break;
            default:
                break;
            }
        }
            break;
        default:
            break;
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_E) {
            inventory.toggle();
            SDL_SetRelativeMouseMode(!inventory.is_open() ? SDL_TRUE : SDL_FALSE);
        }
        if (inventory.is_open()) {
            const auto outer = frect{ 0, 0, static_cast<float>(GetTrueWindowSize().x), static_cast<float>(GetTrueWindowSize().y) };
            inventory.handle_events(event, outer, 0);

            if (event.type == SDL_MOUSEBUTTONDOWN) {
            }
        } else {
            if (event.type == SDL_MOUSEBUTTONDOWN && SDL_GetRelativeMouseMode() == SDL_FALSE && event.button.button == SDL_BUTTON_LEFT) {
                SDL_SetRelativeMouseMode(SDL_TRUE);
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                SDL_SetRelativeMouseMode(SDL_FALSE);
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                player.jump(*this);
            }
            if (event.type == SDL_MOUSEMOTION && SDL_GetRelativeMouseMode() == SDL_TRUE) {
                player.look_right(M_PI * event.motion.xrel / 1000.0f, *this);
                player.look_up(-M_PI * event.motion.yrel / 1000.0f, *this);
            }
        }

        ASSERT_ON_GL_ERROR();
    }

    const float speed = 0.4f;

    if (!inventory.is_open()) {
        const auto keypresses = SDL_GetKeyboardState(NULL);
        if (keypresses[SDL_SCANCODE_A]) {
            player.move_right(-speed, *this);
        }
        if (keypresses[SDL_SCANCODE_D]) {
            player.move_right(speed, *this);
        }
        if (keypresses[SDL_SCANCODE_S])  {
            player.move_forward(-speed, *this);
        }
        if (keypresses[SDL_SCANCODE_W])  {
            player.move_forward(speed, *this);
        }
        if (keypresses[SDL_SCANCODE_F3]) {
            player.toggle_debug_mode(*this);
        }
    }
    player.apply_gravity(*this);
}

bool World::intersects_block(const glm::vec3 &pos, const AABB &aabb) const {
    for (const auto &chunk : this->chunks) {
        if (chunk.intersects(pos, aabb)) {
            return true;
        }
    }
    return false;
}

glm::vec3 World::try_move_to(const glm::vec3 &pos, const glm::vec3 &delta_pos, const AABB &aabb) const {
    glm::vec3 new_pos = pos;
    glm::vec3 components[] = { glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1) };
    for (int i = 0; i < 3; ++i) {
        if (!this->intersects_block(pos + delta_pos * components[i], AABB(1.f, 2.f, 1.f))) {
            new_pos += delta_pos * components[i];
        }
    }
    return new_pos;
}

void World::draw() {
    ASSERT_ON_GL_ERROR();
    glBindBuffer(GL_UNIFORM_BUFFER, globals_3d_ubo);
    glm::mat4 view = player.camera.view_matrix();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    ASSERT_ON_GL_ERROR();

    shader.apply_bindings({
        { "blocks", blocks_texture }
    });

    shader.use();

    for (uint32_t i = 0; i < chunks.size(); ++i) {
        ASSERT_ON_GL_ERROR();

        shader.retrieve_shader_variable<glm::ivec2>("chunk_pos").set(chunks[i].chunk_pos);
        
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_objects[i]);

        glBindVertexArray(VAO);
        //glDrawArraysInstanced(GL_TRIANGLES, 0, cube_vertices.size() / 5, Chunk::BLOCKS_IN_CHUNK);
        glDrawArrays(GL_POINTS, 0, vertex_buffer_objects_size[i]);

        ASSERT_ON_GL_ERROR();
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    inventory.draw(frect{ 0, 0, static_cast<float>(GetTrueWindowSize().x), static_cast<float>(GetTrueWindowSize().y) }, 0);

    ASSERT_ON_GL_ERROR();
}

void World::generate(uint32_t seed) {
    destroy_chunks();

    siv::PerlinNoise noise(seed);

    const float frequency = 4.f;
    const int32_t octaves = 4;

    const double fx = (Chunk::WIDTH * 5) / frequency;
    const double fy = (Chunk::WIDTH * 5) / frequency;

    for (uint32_t i = 0; i < 5; ++i)
        for (uint32_t j = 0; j < 5; ++j) {

        Chunk &chunk = this->chunks.emplace_back();

        for (auto pos = glm::ivec3(); Chunk::is_within_chunk_bounds(pos); Chunk::loop_through(pos)) {
            BlockType block_id = BlockType::Air;
            assert(0 <= pos.x && pos.x < Chunk::WIDTH);
            assert(0 <= pos.z && pos.z < Chunk::WIDTH);

            double r_noise = noise.accumulatedOctaveNoise2D_0_1((pos.x + (i * Chunk::WIDTH))  / fx, (pos.z + (j * Chunk::WIDTH)) / fy, octaves);
            const uint8_t height = static_cast<uint8_t>(std::clamp(r_noise * 128, 0.0, 255.0));
            if (pos.y <= height) {
                if (pos.y == height) {
                    block_id = BlockType::Grass;
                } else if (pos.y + 4 > height) {
                    block_id = BlockType::Dirt;
                } else {
                    block_id = BlockType::Stone;
                }
            }
            chunk.SetBlock(pos, block_id);
        }
        chunk.chunk_pos = glm::ivec2(i, j);
    }
}



void World::load(const std::string &path) {
    destroy_chunks();
    this->save_name = path;

    try {
        std::ifstream file(path, std::ios_base::binary);
        file.exceptions(~std::ios_base::goodbit);

        const auto pad16 = [&file]() {
            while (file.tellg() % 16 != 0) {
                read_binary<uint8_t>(file);
            }
        };

        const uint32_t version = read_binary<uint32_t>(file);
        assert(version == World::WORLD_VERSION);
        const uint32_t chunks_count = read_binary<uint32_t>(file);

        this->chunks.reserve(chunks_count);
        std::vector<uint32_t> chunk_positions;
        chunk_positions.reserve(chunks_count);

        for (uint32_t i = 0; i < chunks_count; ++i) {
            chunk_positions.push_back(read_binary<uint32_t>(file));
        }

        pad16();

        for (uint32_t i = 0; i < chunks_count; ++i) {
            Chunk &chunk = this->chunks.emplace_back();
            file.seekg(chunk_positions[i], std::ios_base::beg);

            const int32_t chunk_pos_x = read_binary<int32_t>(file);
            const int32_t chunk_pos_y = read_binary<int32_t>(file);
            chunk.chunk_pos = glm::ivec2(chunk_pos_x, chunk_pos_y);

            const uint64_t blocks_in_chunk = read_binary<uint64_t>(file);
            assert(blocks_in_chunk == Chunk::BLOCKS_IN_CHUNK);

            pad16();
            for (auto pos = glm::ivec3(); Chunk::is_within_chunk_bounds(pos); Chunk::loop_through(pos)) {
                uint32_t block = read_binary<uint32_t>(file);
                assert(BLOCK_MIN <= block && block < BLOCK_MAX);
                chunk.SetBlock(pos, static_cast<BlockType>(block));
            }
            pad16();
        }

        file.close();
    } catch(std::ios_base::failure &error) {
        std::cout << "File could not open: " << error.what() << std::endl;
        std::cout << "Generating new world instead" << std::endl;

        World::generate();
    }

    ASSERT_ON_GL_ERROR();

    this->vertex_buffer_objects.resize(this->chunks.size());
    this->vertex_buffer_objects_size.resize(this->chunks.size());
    glGenBuffers(chunks.size(), vertex_buffer_objects.data());
    std::vector<glm::uvec4> data;
    for (uint32_t i = 0; i < vertex_buffer_objects.size(); ++i) {
        data.clear();
        gen_chunk_blocks(chunks[i], data);
        vertex_buffer_objects_size[i] = data.size();

        glBindVertexArray(this->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_objects[i]);

        glBufferData(GL_ARRAY_BUFFER, data.size(), data.data(), GL_STATIC_DRAW);
        glVertexAttribIPointer(0, 4, GL_UNSIGNED_INT, sizeof(glm::uvec4), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    ASSERT_ON_GL_ERROR();
}
void World::save(const std::string &path) const {
    try {
        std::ofstream file(path, std::ios_base::binary);
        file.exceptions(~std::ios_base::goodbit);

        const auto pad16 = [&file]() {
            while (file.tellp() % 16 != 0) {
                write_binary<uint8_t>(file, static_cast<uint8_t>(0));
            }
        };

        assert(chunks.size() > 0);

        write_binary<uint32_t>(file, WORLD_VERSION);
        write_binary<uint32_t>(file, this->chunks.size());

        const uint32_t chunk_pos_start = file.tellp();

        for (uint32_t i = 0; i < this->chunks.size(); ++i) {
            write_binary<uint32_t>(file, 0xffffffff);
        }

        pad16();

        for (uint32_t i = 0; i < this->chunks.size(); ++i) {
            const uint32_t curr_chunk_pos = file.tellp();
            file.seekp(chunk_pos_start + i * sizeof(uint32_t), std::ios_base::beg);
            write_binary<uint32_t>(file, curr_chunk_pos);
            file.seekp(curr_chunk_pos, std::ios_base::beg);

            write_binary<int32_t>(file, chunks[i].chunk_pos.x);
            write_binary<int32_t>(file, chunks[i].chunk_pos.y);
            write_binary<uint64_t>(file, Chunk::BLOCKS_IN_CHUNK);

            file.flush();

            pad16();

            file.flush();

            for (auto pos = glm::ivec3(); Chunk::is_within_chunk_bounds(pos) ;Chunk::loop_through(pos)) {
                write_binary<uint32_t>(file, static_cast<uint32_t>(this->chunks[i].GetBlock(pos)));
            }


            pad16();
        }

        file.close();

    } catch (std::ios_base::failure &error) {
        std::cout << "File could not be written to: " << error.what() << std::endl;
    }
}


void World::gen_chunk_blocks(const Chunk &chunk, std::vector<glm::uvec4> &data) {
    assert(data.empty());

    const std::array<glm::ivec3, 6> offsets = {
        glm::ivec3(0, 0, 1),
        glm::ivec3(0, 1, 0),
        glm::ivec3(1, 0, 0),
        glm::ivec3(0, 0, -1),
        glm::ivec3(0, -1, 0),
        glm::ivec3(-1, 0, 0),
    };

    const glm::ivec3 bounds = { Chunk::WIDTH, Chunk::HEIGHT, Chunk::LENGTH };
    for (glm::ivec3 pos = glm::ivec3(0, 0, 0); 
        array3d_index_in_bounds(pos, bounds); 
        array3d_iterate_index(pos, bounds)
    ) {
        if (
            pos.x == 0 || 
            pos.y == 0 || 
            pos.z == 0 || 
            pos.x + 1 == Chunk::WIDTH ||
            pos.y + 1 == Chunk::HEIGHT ||
            pos.z + 1 == Chunk::LENGTH
        ) {
            data.push_back(glm::uvec4(
                static_cast<uint32_t>(pos.x),
                static_cast<uint32_t>(pos.y),
                static_cast<uint32_t>(pos.z),
                static_cast<uint32_t>(chunk.GetBlock(pos))
            ));
            continue;
        }

        for (auto offset : offsets) {
            const glm::ivec3 adjacent_block = offset + pos;
            BlockType block = chunk.GetBlock(adjacent_block);
            if (block == BlockType::Air) {
                data.push_back(glm::uvec4(
                    static_cast<uint32_t>(pos.x),
                    static_cast<uint32_t>(pos.y),
                    static_cast<uint32_t>(pos.z),
                    static_cast<uint32_t>(chunk.GetBlock(pos))
                ));
                break;
            }
        }
    }
}