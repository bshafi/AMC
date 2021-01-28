#version 410 core

out vec4 FragColor;

in vec2 texture_coordinates;
flat in uint tex_id;

uniform sampler2D textures[8u];

void main() {
    if (tex_id > 8u) {
        FragColor = vec4(255, 0, 0, 1.0f);
    }
    vec4 col = texture(textures[tex_id], texture_coordinates);
    if (col.a < .1) {
        discard;
    }
    FragColor = col;
}
