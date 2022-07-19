#version 410

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec2 texture_coordinates;
layout (location = 2) in uint texture_id;

layout (std140) uniform globals_3d {
    mat4x4 view;
    mat4x4 projection;
    mat4x4 model;
};

out vec2 tex_coords;
flat out uint tex_id;

void main() {
    tex_id = texture_id;
    tex_coords = texture_coordinates;

    gl_Position = projection * view * model * vec4(vertex_position, 1.f);
}