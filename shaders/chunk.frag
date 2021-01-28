#version 410 core

out vec4 FragColor;

uniform vec3 outColor;

in vec2 texCoords;
uniform sampler2D orientation;

uniform sampler2D blocks;

flat in uint ablock_id;

void main() {
    // Number of blocks implemented
    uint BLOCK_TYPE_COUNT = 4u;

    // block_id of zero means a empty space
    if (ablock_id == 0u) {
        discard;
    }
    // This forces glsl to not exclude variables which would make the application crash
    float zero = 0 * (length(outColor) + length(texCoords) + length(texture(orientation, texCoords)) + length(texture(blocks, texCoords)) + float(BLOCK_TYPE_COUNT) + float(ablock_id));
    
    float tex_y = ((float(BLOCK_TYPE_COUNT) - float(ablock_id) + texCoords.y) / 4);
    FragColor = texture(blocks, vec2(texCoords.x, tex_y)) + vec4(0, 0, 0, 0) * zero;
}