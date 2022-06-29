#include <fstream>

#include "PerlinNoise.hpp"

#include "gl_helper.hpp"
#include "hello_cube.hpp"

#include "world.hpp"
#include "chunk.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"



glm::vec3 PhysicalWorld::try_move_to(const glm::vec3 &pos, const glm::vec3 &delta_pos, const AABB &aabb) const {
    glm::vec3 new_pos = pos;
    glm::vec3 components[] = { glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1) };
    for (int i = 0; i < 3; ++i) {
        if (!this->intersects_block(pos + delta_pos * components[i], AABB(1.f, 2.f, 1.f))) {
            new_pos += delta_pos * components[i];
        }
    }
    return new_pos;
}
bool PhysicalWorld::intersects_block(const glm::vec3 &pos, const AABB &aabb) const {
    for (const auto &[chunk_pos, chunk] : this->chunks) {
        if (chunk.intersects(pos, aabb)) {
            return true;
        }
    }
    return false;
}

BlockHit::Face get_hit_face(const glm::vec3 &hit_pos, const glm::ivec3 &block_location) {
    const glm::vec3 world_location = glm::vec3(block_location) + 0.5f * glm::vec3(1, 1, 1);
    const glm::vec3 hit_offset = hit_pos - world_location;
    int max_dir = 0;
    if (fabs(hit_offset[max_dir]) < fabs(hit_offset.y)) {
        max_dir = 1;
    }
    if (fabs(hit_offset[max_dir]) < fabs(hit_offset.z)) {
        max_dir = 2;
    }
    switch (max_dir) {
    case 0: {
            if (hit_offset.x < 0) {
                return BlockHit::Face::NegX;
            } else {
                return BlockHit::Face::PosX;
            }
        }
        break;
    case 1: {
            if (hit_offset.y < 0) {
                return BlockHit::Face::NegY;
            } else {
                return BlockHit::Face::PosY;
            }
        }
        break;
    case 2: {
            if (hit_offset.z < 0) {
                return BlockHit::Face::NegZ;
            } else {
                return BlockHit::Face::PosZ;
            }
        }
        break;
    default:
        assert(false);
    }
}


std::optional<BlockHit> PhysicalWorld::GetBlockFromRay(const Ray &ray) {
    float min_t = std::numeric_limits<float>::infinity();
    std::optional<BlockHit> block = std::nullopt;
    glm::ivec3 location;
    for (auto &[chunk_pos, chunk] : chunks) {
        std::optional<float> hit = ray.cast(chunk, 100.0f);
        if (!hit.has_value()) {
            continue;
        }
        glm::vec3 hit_pos = (*hit) * ray.direction + ray.endpoint;
        location = get_hit_block(chunk, ray.direction, hit_pos);
        assert(chunk_contains(chunk, location));
        if ((*hit) < min_t) {
            min_t = *hit;
            block = BlockHit{ chunk_pos, location, get_hit_face(hit_pos, location) };
        }
    }
    return block;
}
BlockType PhysicalWorld::GetBlock(const BlockHit &block_handle) {
    return chunks[block_handle.chunk_pos].GetBlock(block_handle.block_pos);
}
void PhysicalWorld::SetBlock(const BlockHit &block_handle, BlockType type) {
    chunks[block_handle.chunk_pos].GetBlock(block_handle.block_pos) = type;
}


void PhysicalWorld::handle_events(const std::vector<SDL_Event> &events, float delta_time_s) {
    for (const auto &event : events) {
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
                player.look_right(0.25f * M_PI * event.motion.xrel * delta_time_s, *this);
                player.look_up(-0.25f * M_PI * event.motion.yrel * delta_time_s, *this);
            }
        }
    }


    const float speed = 100.f * delta_time_s;

    if (!inventory.is_open()) {
        const auto keypresses = SDL_GetKeyboardState(NULL);
        if (keypresses[SDL_SCANCODE_A]) {
            player.move_right(-speed, *this);
        }
        if (keypresses[SDL_SCANCODE_D]) {
            player.move_right(speed, *this);
        }
        if (keypresses[SDL_SCANCODE_S]) {
            player.move_forward(-speed, *this);
        }
        if (keypresses[SDL_SCANCODE_W]) {
            player.move_forward(speed, *this);
        }
        if (keypresses[SDL_SCANCODE_F3]) {
            player.toggle_debug_mode();
        }
    }
    player.apply_gravity(*this, delta_time_s);

    const uint32_t mouse_button_state = SDL_GetMouseState(nullptr, nullptr);
    if (mouse_button_state & SDL_BUTTON(SDL_BUTTON_LEFT) || mouse_button_state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
        auto new_block = GetBlockFromRay(Ray{ player.camera.pos(), player.camera.forward() });
        if (selected_block != std::nullopt &&
            new_block->chunk_pos == selected_block->chunk_pos &&
            new_block->block_pos == selected_block->block_pos
        ) {
            this->selected_block = new_block;
            selected_block_damage -= (10 * BLOCK_DURABILITY) / FPS;
            if (selected_block_damage < 0) {
                SetBlock(*selected_block, BlockType::Air);
                /* TODO: FIXME When blocks change request a change in the renderer somehow
                auto loc = meshes.find(new_block->chunk_pos);
                if (loc != meshes.end()) {
                    loc->second = MeshBuffer(BlockMesh::Generate(chunks[new_block->chunk_pos]));
                }
                */
            }
        } else {
            this->selected_block = new_block;
            selected_block_damage = BLOCK_DURABILITY;
        }
    }
}

PhysicalWorld::PhysicalWorld() {
    selected_block_damage = BLOCK_DURABILITY;
}
PhysicalWorld::~PhysicalWorld() {
    this->save(save_name);
}


void PhysicalWorld::generate(uint32_t seed) noexcept {
    this->chunks.clear();

    siv::PerlinNoise noise(seed);
    
    const double fx = (Chunk::CHUNK_WIDTH * 5) / frequency;
    const double fy = (Chunk::CHUNK_WIDTH * 5) / frequency;

    for (uint32_t i = 0; i < 5; ++i)
        for (uint32_t j = 0; j < 5; ++j) {
        Chunk chunk;

        for (auto pos = glm::ivec3(); Chunk::is_within_chunk_bounds(pos); Chunk::loop_through(pos)) {
            BlockType block_id = BlockType::Air;
            assert(0 <= pos.x && pos.x < Chunk::CHUNK_WIDTH);
            assert(0 <= pos.z && pos.z < Chunk::CHUNK_WIDTH);

            double r_noise = noise.accumulatedOctaveNoise2D_0_1((pos.x + (i * Chunk::CHUNK_WIDTH))  / fx, (pos.z + (j * Chunk::CHUNK_WIDTH)) / fy, octaves);
            const uint8_t height = static_cast<uint8_t>(std::clamp(r_noise * 128, 0.0, 255.0));
            if (pos.y <= height) {
                if (pos.y == height) {
                    block_id = (BlockType::Grass);
                } else if (pos.y + 4 > height) {
                    block_id = (BlockType::Dirt);
                } else {
                    block_id = (BlockType::Stone);
                }
            }
            chunk.GetBlock(pos) = block_id;
        }
        chunk.chunk_pos = glm::ivec2(i, j);
        chunks.emplace(std::make_pair<glm::ivec2, Chunk>(glm::ivec2(chunk.chunk_pos), std::move(chunk)));
    }
}



void PhysicalWorld::load(const std::string &path) {
    this->chunks.clear();
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
        assert(version == PhysicalWorld::WORLD_VERSION);
        const uint32_t chunks_count = read_binary<uint32_t>(file);

        //this->chunks.reserve(chunks_count);
        std::vector<uint32_t> chunk_positions;
        chunk_positions.reserve(chunks_count);

        for (uint32_t i = 0; i < chunks_count; ++i) {
            chunk_positions.push_back(read_binary<uint32_t>(file));
        }

        pad16();

        for (uint32_t i = 0; i < chunks_count; ++i) {
            Chunk chunk;
            file.seekg(chunk_positions[i], std::ios_base::beg);

            const int32_t chunk_pos_x = read_binary<int32_t>(file);
            const int32_t chunk_pos_y = read_binary<int32_t>(file);
            chunk.chunk_pos = glm::ivec2(chunk_pos_x, chunk_pos_y);

            const uint64_t blocks_in_chunk = read_binary<uint64_t>(file);
            assert(blocks_in_chunk == Chunk::BLOCKS_IN_CHUNK);

            pad16();
            for (auto pos = glm::ivec3(); Chunk::is_within_chunk_bounds(pos); Chunk::loop_through(pos)) {
                chunk.GetBlock(pos) = static_cast<BlockType>(read_binary<uint32_t>(file));
            }
            pad16();
            chunks.emplace(std::make_pair<glm::ivec2, Chunk>(glm::ivec2(chunk.chunk_pos), std::move(chunk)));
        }

        file.close();
    } catch(std::ios_base::failure &error) {
        std::cout << "File could not open: " << error.what() << std::endl;
        std::cout << "Generating new world instead" << std::endl;

        return PhysicalWorld::generate();
    }
}
void PhysicalWorld::save(const std::string &path) const {
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
        {
            uint32_t i = 0;
            for (const auto &[chunk_pos, chunk] : chunks) {
                const uint32_t curr_chunk_pos = file.tellp();
                file.seekp(chunk_pos_start + i * sizeof(uint32_t), std::ios_base::beg);
                write_binary<uint32_t>(file, curr_chunk_pos);
                file.seekp(curr_chunk_pos, std::ios_base::beg);

                write_binary<int32_t>(file, chunk.chunk_pos.x);
                write_binary<int32_t>(file, chunk.chunk_pos.y);
                write_binary<uint64_t>(file, Chunk::BLOCKS_IN_CHUNK);

                file.flush();

                pad16();

                file.flush();

                for (auto pos = glm::ivec3(); Chunk::is_within_chunk_bounds(pos) ;Chunk::loop_through(pos)) {
                    write_binary<uint32_t>(file, static_cast<uint32_t>(chunk.GetBlock(pos)));
                }

                pad16();
                ++i;
            }
        }

        file.close();

    } catch (std::ios_base::failure &error) {
        std::cout << "File could not be written to: " << error.what() << std::endl;
    }
}

void RenderWorld::handle_events(const std::vector<SDL_Event> &events) {
    for (const auto &event : events) { 
        if (event.type == SDL_WINDOWEVENT_RESIZED) {
            glViewport(0, 0, event.window.data1, event.window.data2);
            glBindBuffer(GL_UNIFORM_BUFFER, globals_3d_ubo);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, nullptr, GL_DYNAMIC_DRAW);
            glm::mat4 projection = glm::perspective(static_cast<float>(M_PI / 4), static_cast<float>(event.window.data1) / event.window.data2, 0.1f, 100.f);
            glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
        }
        ASSERT_ON_GL_ERROR();
    }
}
void RenderWorld::draw(PhysicalWorld &phys) {
    for (const auto &[key, chunk] : phys.chunks) {
        if (meshes.find(key) == meshes.end()) {
            meshes.emplace(std::make_pair<glm::ivec2, MeshBuffer>(glm::ivec2(chunk.chunk_pos), (MeshBuffer(BlockMesh::Generate(chunk)))));
        }
    }

    ASSERT_ON_GL_ERROR();
     
    glBindBuffer(GL_UNIFORM_BUFFER, globals_3d_ubo);
    glm::mat4 view = phys.player.camera.view_matrix();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    ASSERT_ON_GL_ERROR();

    shader.bind_texture_to_sampler_2D({
        { "orientation", orientation_texture },
        { "blocks", blocks_texture }
    });
    shader.bind_texture_to_sampler_2D({
        { "orientation", orientation_texture },
        { "blocks", blocks_texture }
    });

    shader.use();

    float RENDER_DISTANCE = PhysicalWorld::RENDER_DISTANCE * Chunk::CHUNK_WIDTH;

    for (auto &[chunk_pos, mesh_buffer] : this->meshes) {
        if (glm::length(glm::vec2(phys.player.position.x, phys.player.position.z) - glm::vec2(chunk_pos.x * Chunk::CHUNK_WIDTH, chunk_pos.y * Chunk::CHUNK_WIDTH)) >= RENDER_DISTANCE) {
            continue;
        }
        shader.retrieve_shader_variable<glm::ivec2>("chunk_pos").set(chunk_pos);
        if (phys.selected_block != std::nullopt && phys.selected_block->chunk_pos == chunk_pos) {
            shader.retrieve_shader_variable<glm::ivec3>("selected_block").set(phys.selected_block->block_pos);
        } else {
            shader.retrieve_shader_variable<glm::ivec3>("selected_block").set(glm::ivec3(1, 1, 1) * INT32_MIN);
        }

        mesh_buffer.draw();
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    phys.inventory.draw(frect{ 0, 0, static_cast<float>(GetTrueWindowSize().x), static_cast<float>(GetTrueWindowSize().y) }, 0);

    ASSERT_ON_GL_ERROR();

#ifdef ENABLE_IMGUI
    ImGui::Begin("World");
    ImGui::DragFloat("frequency", &phys.frequency, 1.0f, 10.0f);
    ImGui::DragInt("octaves", &phys.octaves, 0, 20);
    if (ImGui::Button("regenerate")) {
        phys.generate();
    }
    if (ImGui::Button("toggle debug mode")) {
        phys.player.debug_mode = !phys.player.debug_mode;
    }
    ImGui::DragFloat("gravity", &Player::gravity, 0.0f, 10.0f);
    ImGui::DragFloat("movement_speed", &Player::movement_speed, 0.0f, 10.0f);
    ImGui::DragFloat("jump_speed", &Player::jump_speed, 0.0f, 10.0f);
    if (phys.selected_block != std::nullopt) {
        int block_type = static_cast<int>(phys.GetBlock(*phys.selected_block));
        ImGui::InputInt("block type", &block_type);
        int block_pos[] = { phys.selected_block->block_pos.x, phys.selected_block->block_pos.y, phys.selected_block->block_pos.z };
        int chunk_pos[] = { phys.selected_block->chunk_pos.x, phys.selected_block->chunk_pos.y };
        ImGui::InputInt3("block_pos", block_pos);
        ImGui::InputInt2("chunk_pos", chunk_pos);
        float block_damage = static_cast<float>(phys.selected_block_damage) / phys.BLOCK_DURABILITY;
        ImGui::InputFloat("block_damage", &block_damage);
    } else {
        ImGui::LabelText("No block selected", "");
    }
    ImGui::End();
#endif
}

RenderWorld::RenderWorld(PhysicalWorld &phys) : 
    orientation_texture("resources/hello_cube_orientation.png"),
    blocks_texture("resources/blocks.png"),
    shader("shaders/block.vert", "shaders/chunk.frag") {


    ASSERT_ON_GL_ERROR();

    shader.use();
    shader.bind_texture_to_sampler_2D({
        { "orientation", orientation_texture },
        { "blocks", blocks_texture }
    });

    ASSERT_ON_GL_ERROR();

    glGenBuffers(1, &globals_3d_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, globals_3d_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, nullptr, GL_STATIC_DRAW);
    glm::mat4 view = phys.player.camera.view_matrix();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
    glm::mat4 projection = glm::perspective(static_cast<float>(M_PI / 4), 640.f / 480.f, 0.1f, 100.f);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));

    this->shader.bind_UBO("globals_3d", 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, globals_3d_ubo);

    this->shader.bind_UBO("globals_3d", 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, globals_3d_ubo);

    ASSERT_ON_GL_ERROR();
}
RenderWorld::~RenderWorld() {
    ASSERT_ON_GL_ERROR();

    glDeleteBuffers(1, &globals_3d_ubo);

    ASSERT_ON_GL_ERROR();
}