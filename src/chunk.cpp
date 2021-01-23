#include <iostream>

#include "gl_helper.hpp"
#include "hello_cube.hpp"
#include "chunk.hpp"

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