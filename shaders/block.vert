#version 410

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec2 h_tex_coords;
layout (location = 2) in uint h_block_id;
layout (location = 3) in ivec3 orignal_location;

#define CHUNK_WIDTH 16

layout (std140) uniform globals_3d {
    mat4x4 view;
    mat4x4 projection;
};

uniform ivec2 chunk_pos;
uniform ivec3 selected_block;

flat out uint block_id;
out vec2 tex_coords;
flat out uint is_selected_block;

void main() {
    block_id = h_block_id;
    tex_coords = h_tex_coords;
    if (orignal_location == selected_block) {
        is_selected_block = 1;
    } else {
        is_selected_block = 0;
    }

    vec3 obj_pos = vertex_position.xyz + vec3(chunk_pos.x, 0, chunk_pos.y) * 16;
    gl_Position = projection * view * vec4(obj_pos, 1.0f);
}