#include <iostream>

#include "gl_helper.hpp"
#include "hello_cube.hpp"
#include "chunk.hpp"

std::optional<unsigned> OldChunk::cube_VBO = std::nullopt;
std::optional<Shader> OldChunk::cube_shader_program = std::nullopt;
std::optional<Texture> OldChunk::cube_texture = std::nullopt;
std::optional<Texture> OldChunk::blocks_texture = std::nullopt;

constexpr glm::vec3 INITIAL_CAMERA_POS = glm::vec3(0, 0, -30);
constexpr glm::vec3 INITIAL_CAMERA_ROT = glm::vec3(0, 0, 0);
constexpr glm::ivec3 INITIAL_CHUNK_POS = glm::ivec3(0, 0, 0);
constexpr float INITIAL_ASPECT_RATIO = 640.0f / 480.0f;

// TODO: Make the constructor specify the height and the location of the chunk
OldChunk::OldChunk(uint32_t height) : slices(height), pitch{0.0f}, yaw{-M_PI_2}, camera_pos{glm::vec3()} {
    for (int i = 0; i < height; ++i) {
        slices.push_back(Slice());
    }

    if (!cube_VBO.has_value()) {
        glGenBuffers(1, &*cube_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, *cube_VBO);
        glBufferData(GL_ARRAY_BUFFER, cube_vertices.size() * sizeof(decltype(cube_vertices)::value_type), cube_vertices.data(), GL_STATIC_DRAW);
    }
    if (!cube_shader_program.has_value()) {
        cube_shader_program.emplace("shaders/chunk.vert", "shaders/chunk.frag");
    }
    if (!cube_texture.has_value()) {
        cube_texture.emplace("resources/hello_cube_orientation.png");
    }
    if (!blocks_texture.has_value()) {
        blocks_texture.emplace("resources/blocks.png");
    }

    cube_shader_program->use();
    chunk_location = cube_shader_program->retrieve_shader_variable<glm::ivec3>("chunk_pos");
    chunk_location.set(INITIAL_CHUNK_POS);

    this->projection = cube_shader_program->retrieve_shader_variable<glm::mat4>("projection");
    this->projection.set(glm::perspective(static_cast<float>(M_PI / 4), 640.f / 480.f, 0.1f, 100.f));
    this->view = cube_shader_program->retrieve_shader_variable<glm::mat4>("view");
    this->view.set(glm::lookAt(camera_pos, camera_pos + forward, glm::vec3(0, 1, 0)));

    cube_shader_program->bind_texture_to_sampler_2D({
            { "orientation", *cube_texture },
            { "blocks", *blocks_texture }
    });

    glGenBuffers(1, &this->block_ids_VBO);

    glGenVertexArrays(1, &this->VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, *cube_VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(decltype(cube_vertices)::value_type), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(decltype(cube_vertices)::value_type), (void*)(3 * sizeof(decltype(cube_vertices)::value_type)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, block_ids_VBO);
    glBufferData(GL_ARRAY_BUFFER, slices.size() * sizeof(decltype(slices)::value_type), slices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(unsigned), (void*)0);
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(2);
}


void OldChunk::draw() {
    cube_shader_program->use();
    this->view.set(glm::lookAt(camera_pos, camera_pos + forward, glm::vec3(0, 1, 0)));
    this->projection.set(this->projection.get());

    glBindVertexArray(VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, cube_vertices.size() / 5, slices.size() * 256);
}

void OldChunk::update(const std::vector<SDL_Event> &events) {
    const auto keypresses = SDL_GetKeyboardState(NULL);

    glm::vec3 right = glm::cross(forward, glm::vec3(0, 1, 0));
    glm::vec3 up = glm::cross(forward, right);

    if (keypresses[SDL_SCANCODE_A]) {
       camera_pos -= right * 0.1f;
    }
    if (keypresses[SDL_SCANCODE_D]) {
        camera_pos += right * 0.1f;
    }
    if (keypresses[SDL_SCANCODE_S])  {
        camera_pos -= 0.1f * forward;
    }
    if (keypresses[SDL_SCANCODE_W])  {
        camera_pos += 0.1f * forward;
    }
    if (keypresses[SDL_SCANCODE_I]) {
    }
    if (keypresses[SDL_SCANCODE_K]) {
    }
    if (keypresses[SDL_SCANCODE_LEFT]) {
    }
    if (keypresses[SDL_SCANCODE_RIGHT]) {
    }

    for (const auto event : events) {
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
        if (event.type == SDL_MOUSEMOTION && SDL_GetRelativeMouseMode() == SDL_TRUE) {
            yaw += (M_PI * event.motion.xrel) / 1000.0f;
            pitch -= (M_PI * event.motion.yrel) / 1000.0f;
        }
    }

    forward = glm::normalize(glm::vec3(
        cos(yaw) * cos(pitch),
        sin(pitch),
        sin(yaw) * cos(pitch)
    ));
}
Chunk::Chunk() : 
    orientation_texture{ "resources/hello_cube_orientation.png" },
    blocks_texture{ "resources/blocks.png" }, 
    shader{ "shaders/chunk.vert", "shaders/chunk.frag" } {

    ASSERT_ON_GL_ERROR();

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    ASSERT_ON_GL_ERROR();

    glGenBuffers(1, &cube_vertices_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vertices_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(decltype(cube_vertices)::value_type) * cube_vertices.size(), cube_vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    ASSERT_ON_GL_ERROR();

    glGenBuffers(1, &chunk_block_ids_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, chunk_block_ids_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(BlockIDType) * Chunk::BLOCKS_IN_CHUNK, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(BlockIDType), (void*)0);
    // TODO: Implement proper terrain generation
    for (unsigned y = 0; y < 256; ++y)
        for (unsigned z = 0; z < 16; ++z)
            for (unsigned x = 0; x < 16; ++x) {
                BlockIDType id = 0;
                if (y < 10) {
                    id = y % 4 + 1;
                }
                glBufferSubData(GL_ARRAY_BUFFER, sizeof(BlockIDType) * (x + 16 * (z + y * 16)), sizeof(BlockIDType), &id);
                ASSERT_ON_GL_ERROR();
            }

    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(2);

    ASSERT_ON_GL_ERROR();

    shader.use();
    shader.bind_texture_to_sampler_2D({
        { "orientation", orientation_texture },
        { "blocks", blocks_texture }
    });

    ASSERT_ON_GL_ERROR();
}
Chunk::~Chunk() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &chunk_block_ids_VBO);
    glDeleteBuffers(1, &cube_vertices_VBO);
}


void Chunk::draw() {
    shader.use();
    shader.retrieve_shader_variable<unsigned>("block_type_count").set(4);

    glBindVertexArray(VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, cube_vertices.size() / 5, Chunk::BLOCKS_IN_CHUNK);

    ASSERT_ON_GL_ERROR();
}