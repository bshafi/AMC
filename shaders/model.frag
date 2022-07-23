#version 410 core

out vec4 FragColor;

in vec2 tex_coords;
flat in uint tex_id;

uniform sampler2D textures[16];

void main() {
    FragColor = texture(textures[tex_id], tex_coords) * 1 + vec4(1.0, 0.0, 0.0, 1.0) * 0;
}