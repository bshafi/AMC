#version 410

layout (location = 0) in uvec4 pos;

flat out uint h_block_id;

void main() {
    h_block_id = pos.w;
    gl_Position = vec4(vec3(pos.xyz), 1.0f);
}