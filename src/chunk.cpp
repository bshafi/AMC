#include "standard.hpp"

#include "hello_cube.hpp"
#include "chunk.hpp"

std::optional<unsigned> Chunk::cube_VBO = std::nullopt;
std::optional<Shader> Chunk::cube_shader_program = std::nullopt;
std::optional<Texture> Chunk::cube_texture = std::nullopt;

constexpr glm::vec3 INITIAL_CAMERA_POS = glm::vec3(0, 0, -30);
constexpr glm::vec3 INITIAL_CAMERA_ROT = glm::vec3(0, 0, 0);
constexpr glm::ivec3 INITIAL_CHUNK_POS = glm::ivec3(0, 0, 0);
constexpr float INITIAL_ASPECT_RATIO = 640.0f / 480.0f;

// TODO: Make the constructor specify the height and the location of the chunk
Chunk::Chunk(uint32_t height) : slices(height) {
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

    cube_shader_program->use();
    chunk_location = cube_shader_program->retrieve_shader_variable<glm::ivec3>("chunk_pos");
    chunk_location.set(INITIAL_CHUNK_POS);

    camera_pos = cube_shader_program->retrieve_shader_variable<glm::vec3>("camera_pos");
    camera_pos.set(INITIAL_CAMERA_POS);

    camera_rot = cube_shader_program->retrieve_shader_variable<glm::vec3>("camera_rot");
    camera_rot.set(INITIAL_CAMERA_ROT);

    aspect_ratio = cube_shader_program->retrieve_shader_variable<float>("aspect_ratio");
    aspect_ratio.set(INITIAL_ASPECT_RATIO);

    cube_shader_program->bind_texture_to_sampler_2D({
            { "tex", *cube_texture }
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


void Chunk::draw() {
    cube_shader_program->use();
    glBindVertexArray(VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, cube_vertices.size() / 5, slices.size() * 256);
}

// TODO: Remove this
glm::mat4x4 myrotate(glm::vec3 rotation) {
    float a = rotation.z;
    float b = rotation.y;
    float c = rotation.x;


    return glm::transpose(glm::mat4x4(
        cos(a) * cos(b), cos(a) * sin(b) * sin(c) - sin(a) * cos(c), cos(a) * sin(b) * cos(c) + sin(a) * sin(c), 0.0f,
        sin(a) * cos(b), sin(a) * sin(b) * sin(c) + cos(a) * cos(c), sin(a) * sin(b) * cos(c) - cos(a) * sin(c), 0.0f,
                -sin(b),                            cos(b) * sin(c),                            cos(b) * cos(c), 0.0f,
                   0.0f,                                       0.0f,                                       0.0f, 1.0f
    ));
}

void Chunk::update(const std::vector<SDL_Event> &events) {
    cube_shader_program->use();
    const auto keypresses = SDL_GetKeyboardState(NULL);

    const glm::vec3 forward = myrotate(camera_rot.get()) * glm::vec4(0.0, 0.0, 1.0, 1.0f);

    if (keypresses[SDL_SCANCODE_A]) {
        camera_pos.set(camera_pos.get() + glm::vec3(0.1, 0, 0));
    }
    if (keypresses[SDL_SCANCODE_D]) {
        camera_pos.set(camera_pos.get() + glm::vec3(-0.1, 0, 0));
    }
    if (keypresses[SDL_SCANCODE_S])  {
        camera_pos.set(camera_pos.get() + glm::vec3(0, 0.1, 0));
    }if (keypresses[SDL_SCANCODE_W])  {
        //camera_pos.set(camera_pos.get() + glm::vec3(0, -0.1, 0));
        camera_pos.set(camera_pos.get() + forward * -0.1f);
    }
    if (keypresses[SDL_SCANCODE_I]) {
        camera_pos.set(camera_pos.get() + glm::vec3(0, 0, 0.1));
    }
    if (keypresses[SDL_SCANCODE_K]) {
        camera_pos.set(camera_pos.get() + glm::vec3(0, 0, -0.1));
    }
    if (keypresses[SDL_SCANCODE_LEFT]) {
        camera_rot.set(camera_rot.get() + glm::vec3(0, 0.01 * M_PI_2, 0));
    }
    if (keypresses[SDL_SCANCODE_RIGHT]) {
        camera_rot.set(camera_rot.get() + glm::vec3(0, -0.01 * M_PI_2, 0));
    }
}