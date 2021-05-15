#version 410 core

out vec4 FragColor;

uniform vec3 outColor;

in vec2 tex_coords;

uniform sampler2D blocks;

flat in uint block_id;

void main() {
    // Number of blocks implemented
    uint BLOCK_TYPE_COUNT = 4u;

    // block_id of zero means a empty space
    if (block_id == 0u) {
        discard;
    }
    // This forces glsl to not exclude variables which would make the application crash
    float zero = 0 * (length(outColor) + length(tex_coords) + length(texture(blocks, tex_coords)) + float(BLOCK_TYPE_COUNT) + float(block_id));
    
    float tex_y = ((float(BLOCK_TYPE_COUNT) - float(block_id) + tex_coords.y) / 4);
    FragColor = texture(blocks, vec2(tex_coords.x, tex_y)) + vec4(0, 0, 0, 0) * zero;
}