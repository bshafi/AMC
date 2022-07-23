#include <fstream>

#include "PerlinNoise.hpp"

#include "gl_helper.hpp"
#include "hello_cube.hpp"

#include "world.hpp"
#include "chunk.hpp"
#include "entity.hpp"
#include "atomic_ring_buffer.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"

SaveFile::SaveFile(const std::string &s) {
    if (!std::filesystem::exists(s)) {
        std::ofstream temp(s);
    }
    file.open(s, std::ios_base::in | std::ios_base::out | std::ios_base::binary);
    file.exceptions(~std::ios_base::goodbit);

    if (file.seekg(0, std::ios_base::end).tellg() == 0) {
        initialize_new_file();
    }
    file.seekg(std::ios_base::beg);

    read_header();

    bool succeeded = false;
    do {
        succeeded = read_table();
        for (size_t i = 0; i < 16; ++i) {
            skip_chunk();
        }
    } while (succeeded);
}
void SaveFile::initialize_new_file() {
    write_header();
    last_table = file.tellp();

    for (size_t i = 0; i < 16; ++i) {
        write_binary<int32_t>(file, INT_MIN);
        write_binary<int32_t>(file, INT_MIN);
    }
    for (size_t i = 0; i < CHUNK_SIZE * 16; ++i) {
        write_binary<uint8_t>(file, 0);
    }
    file.flush();
}
void SaveFile::pad16() {
    while (file.tellg() % 16 != 0) {
        write_binary<uint8_t>(file, 0);
    }
}
void SaveFile::skip16() {
    while (file.tellg() % 16 != 0) {
        read_binary<uint8_t>(file);
    }
}
void SaveFile::skip_chunk() {
    file.seekg(file.tellg() + std::streamoff(CHUNK_SIZE));
}
bool SaveFile::read_header() {
    auto prev_pos = file.tellg();
    try {
        const uint32_t version = read_binary<uint32_t>(file);
        assert(version == PhysicalWorld::WORLD_VERSION);

        skip16();
        last_table = file.tellg();

    } catch (std::ios_base::failure &error) {
        std::cout << "Failed to read header: " << error.what() << std::endl;
        file.clear();
        file.seekg(prev_pos);
        return false;
    }
    return true;
}
void SaveFile::write_header() {
    auto prev_pos = file.tellg();

    write_binary<uint32_t>(file, PhysicalWorld::WORLD_VERSION);
    pad16();
    file.flush();
}
bool SaveFile::read_table() {
    auto prev_pos = file.tellg();
    try {
        std::array<glm::ivec2, 16> chunks;
        for (size_t i = 0; i < chunks.size(); ++i) {
            chunks[i].x = read_binary<int32_t>(file);
            chunks[i].y = read_binary<int32_t>(file);
        }
        auto cur_pos = file.tellg();
        for (size_t i = 0; i < chunks.size(); ++i) {
            if (chunks[i].x != INT32_MIN && chunks[i].y != INT32_MIN) {
                chunk_locations[chunks[i]] = cur_pos + std::streamoff(i * CHUNK_SIZE);
            }
        }
    } catch (std::ios_base::failure &error) {
        file.clear();
        file.seekg(prev_pos);
        return false;
    }
    last_table = prev_pos;
    return true;
}
// returns false if chunk was not found
bool SaveFile::read_chunk(const glm::ivec2 &pos, Chunk &chunk) {
    auto prev_pos = file.tellg();
    try {
        auto loc = chunk_locations.find(pos);
        if (loc == chunk_locations.end()) {
            return false;
        }
        file.seekg(loc->second);
        for(auto pos = glm::ivec3(); Chunk::is_within_chunk_bounds(pos); Chunk::loop_through(pos)) {
            uint8_t block = read_binary<uint8_t>(file);
            chunk.GetBlock(pos) = static_cast<BlockType>(static_cast<uint32_t>(block));
        }
        chunk.chunk_pos = pos;
    } catch (std::ios_base::failure &error) {
        file.clear();
        file.seekg(prev_pos);
        return false;
    }

    return true;
}

void SaveFile::write_chunk(const Chunk &chunk) {
    auto prev_pos = file.tellp();
    try {
        std::streampos chunk_stream_pos = 0;
        auto loc = chunk_locations.find(chunk.chunk_pos);
        if (loc != chunk_locations.end()) {
            chunk_stream_pos = loc->second;
        } else {
            file.seekg(last_table);
            size_t i = 0;
            for (; i < 16; ++i) {
                glm::ivec2 location = {
                    read_binary<int32_t>(file),
                    read_binary<int32_t>(file)
                };
                if (location.x == INT_MIN && location.y == INT_MIN) {
                    file.seekp(last_table + std::streamoff(i * sizeof(int32_t) * 2));
                    write_binary<int32_t>(file, chunk.chunk_pos.x);
                    write_binary<int32_t>(file, chunk.chunk_pos.y);
                    chunk_stream_pos = last_table + std::streamoff(TABLE_SIZE + CHUNK_SIZE * i);
                    break;
                }
            }
            if (i == 16) {
                file.seekp(last_table + std::streamoff(TABLE_SIZE + CHUNK_SIZE * 16));
                last_table = file.tellp();
                chunk_stream_pos = last_table + std::streamoff(TABLE_SIZE);
                write_binary<int32_t>(file, chunk.chunk_pos.x);
                write_binary<int32_t>(file, chunk.chunk_pos.y);
                for (size_t i = 1; i < 16; ++i) {
                    write_binary<int32_t>(file, INT_MIN);
                    write_binary<int32_t>(file, INT_MIN);
                }
                for (size_t i = 0; i < CHUNK_SIZE * 16; ++i) {
                    write_binary<uint8_t>(file, 0);
                }
            }
            chunk_locations[chunk.chunk_pos] = chunk_stream_pos;
        }

        file.seekp(chunk_stream_pos);


        for (auto pos = glm::ivec3(); Chunk::is_within_chunk_bounds(pos); Chunk::loop_through(pos)) {
            uint8_t block = static_cast<uint8_t>(static_cast<uint32_t>(chunk.GetBlock(pos)));
            write_binary<uint8_t>(file, block);
        }
    } catch (std::ios_base::failure &error) {
        file.clear();
        file.seekp(prev_pos);
        assert(false);
    }
    file.flush();
}

SaveFile::~SaveFile() {
    file.flush();
    file.close();
}

glm::vec3 PhysicalWorld::try_move_to(const glm::vec3 &pos, const glm::vec3 &delta_pos, const AABB &aabb) const {
    glm::vec3 new_pos = pos;
    glm::vec3 components[] = { glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1) };
    for (int i = 0; i < 3; ++i) {
        if (!this->intersects_block(pos + delta_pos * components[i], aabb)) {
            new_pos += delta_pos * components[i];
        }
    }
    return new_pos;
}
bool PhysicalWorld::intersects_block(const glm::vec3 &pos, const AABB &aabb) const {
    for (const auto &[chunk_pos, chunk] : this->chunks) {
        if (chunk->intersects(pos, aabb)) {
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
        std::optional<float> hit = ray.cast(*chunk, 100.0f);
        if (!hit.has_value()) {
            continue;
        }
        glm::vec3 hit_pos = (*hit) * ray.direction + ray.endpoint;
        location = get_hit_block(*chunk, ray.direction, hit_pos);
        assert(chunk_contains(*chunk, location));
        if ((*hit) < min_t) {
            min_t = *hit;
            block = BlockHit{ chunk_pos, location, get_hit_face(hit_pos, location) };
        }
    }
    return block;
}
BlockType PhysicalWorld::GetBlock(const BlockHit &block_handle) {
    return chunks[block_handle.chunk_pos]->GetBlock(block_handle.block_pos);
}
void PhysicalWorld::SetBlock(const BlockHit &block_handle, BlockType type) {
    chunks[block_handle.chunk_pos]->GetBlock(block_handle.block_pos) = type;
}


void PhysicalWorld::update_chunks() {
    {
        ChunkPtr chunk = loader.retrive_chunk();
        if (chunk != nullptr) {
            requested_chunks.erase(chunk->chunk_pos);
            chunks.emplace(std::make_pair<glm::ivec2, ChunkPtr>(glm::ivec2(chunk->chunk_pos), std::move(chunk)));
        }
    }

    glm::ivec2 camera_chunk_pos = Chunk::world_pos_to_chunk_pos(main_camera.pos());

    bool made_request = false;
    for (int i = -5; i <= 5; ++i)
        for (int j =  -5; j <=5; ++j) {
            auto pos = glm::ivec2(i, j) + camera_chunk_pos;
            if (chunks.find(pos) == chunks.end() && requested_chunks.find(pos) == requested_chunks.end()) {
                loader.request_chunk(pos);
                requested_chunks.insert(pos);
                made_request = true;
            }
        }

    for (auto a = chunks.begin(); a != chunks.end();) {
        auto diff = a->first - camera_chunk_pos;
        auto manhattan_dist = abs(diff.x) + abs(diff.y);
        if (manhattan_dist > 10) {
            loader.request_unload(std::move(a->second));
            made_request = true;
            a = chunks.erase(a);
        } else {
            ++a;
        }
    }

    if (made_request) {
        loader.notify_request();
    }
}
void PhysicalWorld::handle_events(const std::vector<SDL_Event> &events, float delta_time_s) {
    update_chunks();

    bool space_pressed = false;
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
        }
    }

    for (auto &entity : entities) {
        if (entity.controller != nullptr) {
            entity.controller->update(entity, *this, events, delta_time_s);
        }
    }

    update_entities(*this, entities, delta_time_s);

    /*

    const uint32_t mouse_button_state = SDL_GetMouseState(nullptr, nullptr);
    if (mouse_button_state & SDL_BUTTON(SDL_BUTTON_LEFT) || mouse_button_state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
        auto new_block = GetBlockFromRay(Ray{ main_camera.pos(), main_camera.forward() });
        if (selected_block != std::nullopt &&
            new_block->chunk_pos == selected_block->chunk_pos &&
            new_block->block_pos == selected_block->block_pos
        ) {
            this->selected_block = new_block;
            selected_block_damage -= (10 * BLOCK_DURABILITY) / FPS;
            if (selected_block_damage < 0) {
                
                SetBlock(*selected_block, BlockType::Air);
            
                TODO: FIXME When blocks change request a change in the renderer somehow
                auto loc = meshes.find(new_block->chunk_pos);
                if (loc != meshes.end()) {
                    loc->second = MeshBuffer(BlockMesh::Generate(chunks[new_block->chunk_pos]));
                }
                
            
            }
        } else {
            this->selected_block = new_block;
            selected_block_damage = BLOCK_DURABILITY;
        }
    }
    */
}


void chunk_loader_loading_fn(std::string path) {
    std::vector<ChunkPtr> empty_chunks;

    SaveFile save_file(path);

    std::vector<ChunkPtr> loading_thread_fullfilled_chunks;

    std::queue<glm::ivec2> temp_requested_chunks;
    std::queue<ChunkPtr> temp_requested_unloads;

    while (true) {
        {
            std::unique_lock lk(ChunkLoader::request_lock);
            ChunkLoader::has_made_request.wait(lk);

            std::swap(temp_requested_chunks, ChunkLoader::requested_chunks);
            std::swap(temp_requested_unloads, ChunkLoader::requested_unloads);
        }

        while (!temp_requested_unloads.empty()) {
            save_file.write_chunk(*temp_requested_unloads.front());
            empty_chunks.emplace_back(std::move(temp_requested_unloads.front()));
            temp_requested_unloads.pop();
        }
        while (!temp_requested_chunks.empty()) {
            glm::ivec2 chunk_pos = temp_requested_chunks.front();
            temp_requested_chunks.pop();
            
            ChunkPtr chunk;
            if (empty_chunks.empty()) {
                chunk = std::make_unique<Chunk>();
            } else {
                chunk = std::move(empty_chunks.back());
                empty_chunks.pop_back();
            }

            if (!save_file.read_chunk(chunk_pos, *chunk)) {
                PhysicalWorld::generate_chunk(chunk_pos, *chunk);
            }
            loading_thread_fullfilled_chunks.push_back(std::move(chunk));
        }

        if (!loading_thread_fullfilled_chunks.empty()) {
            std::unique_lock lk(ChunkLoader::fullfilled_lock);
            ChunkLoader::fullfilled_is_empty.wait(lk);
            std::swap(loading_thread_fullfilled_chunks, ChunkLoader::fullfilled_chunks);
        }
    }
}
ChunkPtr ChunkLoader::retrive_chunk() {
    ChunkPtr chunk;
    if (fullfilled_lock.try_lock()) {
        if (fullfilled_chunks.empty()) {
            fullfilled_is_empty.notify_one();
        } else {
            chunk = std::move(fullfilled_chunks.back());
            fullfilled_chunks.pop_back();
        }

        fullfilled_lock.unlock();
    }

    return chunk;
}
bool ChunkLoader::request_chunk(glm::ivec2 v) {
    if (request_lock.try_lock()) {
        requested_chunks.push(v);
        request_lock.unlock();
        return true;
    }
    return false;
}
bool ChunkLoader::request_unload(ChunkPtr &&p) {
    if (request_lock.try_lock()) {
        requested_unloads.emplace(std::move(p));
        request_lock.unlock();
        return true;
    }
    return false;
}
void ChunkLoader::notify_request() {
    has_made_request.notify_one();
}

ChunkLoader::ChunkLoader(const std::string &path) {
    std::call_once(loading_fn_init_flag, [&path]() {
        loading_thread = std::thread(chunk_loader_loading_fn, path);
    });
}

std::thread ChunkLoader::loading_thread;
std::once_flag ChunkLoader::loading_fn_init_flag;

std::mutex ChunkLoader::fullfilled_lock;
std::condition_variable ChunkLoader::fullfilled_is_empty;
std::vector<ChunkPtr> ChunkLoader::fullfilled_chunks;

std::mutex ChunkLoader::request_lock;
std::condition_variable ChunkLoader::has_made_request;
std::queue<glm::ivec2> ChunkLoader::requested_chunks;
std::queue<ChunkPtr> ChunkLoader::requested_unloads;


PhysicalWorld::PhysicalWorld()
    : loader("saves/save0.hex") {
    selected_block_damage = BLOCK_DURABILITY;

    {
        Entity main_player = *Entity::create("player");
        main_player.controller->init(main_player, *this);
        entities.emplace_back(std::move(main_player));
    }
}
PhysicalWorld::~PhysicalWorld() {
}


void PhysicalWorld::generate_chunk(glm::ivec2 p, Chunk &chunk, uint32_t seed) {
    chunk.chunk_pos = p;
    siv::PerlinNoise noise(seed);
    
    p.x %= 5;
    if (p.x < 0) {
        p.x = 5 - p.x;
    }
    int i = p.x;
    p.y %= 5;
    if (p.y < 0) {
        p.y = 5 - p.y;
    }
    int j = p.y;
    

    const int32_t octaves = 4;
    const float frequency = 1.f;
    const double fx = (Chunk::CHUNK_WIDTH * 5) / frequency;
    const double fy = (Chunk::CHUNK_WIDTH * 5) / frequency;

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
}


void PhysicalWorld::load(const std::string &path) {
    this->chunks.clear();
    this->save_name = path;
    //save_file = SaveFile(path);
    /*
    for (size_t i = 0; i < 5; ++i)
        for (size_t j = 0; j < 5; ++j) {
            std::unique_ptr<Chunk> chunk = std::make_unique<Chunk>();
            if (!save_file.read_chunk(glm::ivec2(i, j), *chunk)) {
                generate_chunk(glm::ivec2(i, j), *chunk);
            }

            chunks.emplace(std::make_pair<glm::ivec2, ChunkPtr>(glm::ivec2(i, j), std::move(chunk)));
        }
    */
}
void RenderWorld::handle_events(const std::vector<SDL_Event> &events) {
    for (const auto &event : events) { 
        if (event.type == SDL_WINDOWEVENT_RESIZED) {
            glViewport(0, 0, event.window.data1, event.window.data2);
            glBindBuffer(GL_UNIFORM_BUFFER, globals_3d_ubo);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, nullptr, GL_DYNAMIC_DRAW);
            auto win_size = GetTrueWindowSize();
            glm::mat4 projection = glm::perspective(static_cast<float>(M_PI / 4), static_cast<float>(win_size.x) / win_size.y, 0.1f, 100.f);
            glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
        }
        ASSERT_ON_GL_ERROR();
    }
}


void RenderWorld::update_chunks(PhysicalWorld &phys) {
    for (auto a = meshes.begin(); a != meshes.end(); ) {
        if (phys.chunks.find(a->first) == phys.chunks.end()) {
            empty_meshes.emplace_back(std::move(a->second));
            a = meshes.erase(a);
        } else {
            ++a;
        }
    }
    for (const auto &[key, chunk] : phys.chunks) {
        if (meshes.find(key) == meshes.end()) {
            if (!empty_meshes.empty()) {
                MeshBuffer buffer = std::move(empty_meshes.back());
                empty_meshes.pop_back();
                buffer.rebuild(*chunk);
                meshes.emplace(std::make_pair<glm::ivec2, MeshBuffer>(glm::ivec2(chunk->chunk_pos), std::move(buffer)));
            } else {
                MeshBuffer buffer(Chunk::BLOCKS_IN_CHUNK);
                buffer.rebuild(*chunk);
                meshes.emplace(std::make_pair<glm::ivec2, MeshBuffer>(glm::ivec2(chunk->chunk_pos), std::move(buffer)));
            }
        }
    }
}
void RenderWorld::draw(PhysicalWorld &phys) {
    update_chunks(phys);

    ASSERT_ON_GL_ERROR();
     
    update_ubo_matrices(globals_3d_ubo, phys.main_camera.view_matrix());
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
        if (glm::length(glm::vec2(phys.main_camera.pos().x, phys.main_camera.pos().z) - glm::vec2(chunk_pos.x * Chunk::CHUNK_WIDTH, chunk_pos.y * Chunk::CHUNK_WIDTH)) >= RENDER_DISTANCE) {
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
    
    //model_renderer.camera = phys.main_camera;
    for (const auto &entity : phys.entities) {
        if (entity.type == EntityType::Player) {
            model_renderer.draw(zombie_model.models, &zombie_texture, phys.main_camera.pos() + phys.main_camera.forward());
        }
    }
    

#ifdef ENABLE_IMGUI
    ImGui::Begin("World");
    ImGui::DragFloat("frequency", &phys.frequency, 1.0f, 10.0f);
    ImGui::DragInt("octaves", &phys.octaves, 0, 20);
    if (ImGui::Button("regenerate")) {
        //phys.generate();
    }
    if (ImGui::Button("toggle debug mode")) {
        //phys.player.debug_mode = !phys.player.debug_mode;
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
    zombie_model("resources/zombie.mcmodel"),
    zombie_texture("resources/zombie.png"),
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

    globals_3d_ubo = generate_ubo(shader, phys.main_camera.view_matrix());
    ASSERT_ON_GL_ERROR();
}
RenderWorld::~RenderWorld() {
    ASSERT_ON_GL_ERROR();

    glDeleteBuffers(1, &globals_3d_ubo);

    ASSERT_ON_GL_ERROR();
}