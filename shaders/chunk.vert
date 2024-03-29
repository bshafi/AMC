#version 410

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 h_tex_coords;
layout (location = 2) in uint h_block_id;

uniform ivec2 chunk_pos;
uniform ivec3 selected_block;

layout (std140) uniform globals_3d {
    mat4x4 view;
    mat4x4 projection;
};

out vec2 tex_coords;
flat out uint block_id;
flat out uint is_selected_block;

#define M_PI 3.1415926535897932384626433832795

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
    tex_coords = h_tex_coords;
    block_id = h_block_id;

    int local_x = gl_InstanceID / (16 * 256);
    int local_y = (gl_InstanceID % (16 * 256)) / 16;
    int local_z = (gl_InstanceID % (16 * 256)) % 16;
    ivec3 local_coordinates = ivec3(local_x, local_y, local_z);
    
    if (local_coordinates == selected_block) {
        is_selected_block = 1;
    } else {
        is_selected_block = 0;
    }

    vec3 object_pos = 16 * ivec3(chunk_pos.x, 0, chunk_pos.y) + local_coordinates;
    vec3 object_rot = vec3(0.0f, 0.0f, 0.0f);
    
    mat4x4 model = translate(object_pos) * rotate(object_rot);
    
    gl_Position = projection * view * model * vec4(pos.xyz, 1.0f);
}