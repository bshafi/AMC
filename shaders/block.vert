#version 410

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec2 h_tex_coords;
layout (location = 2) in ivec3 block_pos;
layout (location = 3) in uvec2 chunk_pos;
layout (location = 4) in uint h_block_id;
layout (location = 5) in vec3 block_rot;

#define CHUNK_WIDTH 16

layout (std140) uniform globals_3d {
    mat4x4 view;
    mat4x4 projection;
};

flat out uint block_id;
out vec2 tex_coords;

mat4x4 rotate(vec3 rotation) {
    float a = rotation.z;
    float b = rotation.y;
    float c = rotation.x;

    return transpose(mat4x4(
        cos(a) * cos(b), cos(a) * sin(b) * sin(c) - sin(a) * cos(c), cos(a) * sin(b) * cos(c) + sin(a) * sin(c), 0.0f,
        sin(a) * cos(b), sin(a) * sin(b) * sin(c) + cos(a) * cos(c), sin(a) * sin(b) * cos(c) - cos(a) * sin(c), 0.0f,
                -sin(b),                            cos(b) * sin(c),                            cos(b) * cos(c), 0.0f,
                   0.0f,                                       0.0f,                                       0.0f, 1.0f
    ));
}
mat4x4 translate(vec3 offset) {
    return transpose(mat4x4(
        1.0f, 0.0f, 0.0f, offset.x,
        0.0f, 1.0f, 0.0f, offset.y,
        0.0f, 0.0f, 1.0f, offset.z,
        0.0f, 0.0f, 0.0f,     1.0f
    ));
}

void main() {
    block_id = h_block_id;
    tex_coords = h_tex_coords;

    vec3 obj_pos = CHUNK_WIDTH * ivec3(chunk_pos.x, 0, chunk_pos.y) + block_pos;
    mat4x4 model = translate(obj_pos) * rotate(block_rot);
    gl_Position = projection * view * model * vec4(vertex_position.xyz, 1.0f);
}