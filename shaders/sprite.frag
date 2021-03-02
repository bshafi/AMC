#version 410 core

out vec4 FragColor;

in vec2 texture_coordinates;

uniform sampler2D tex;

void main() {
    vec4 col = texture(tex, texture_coordinates);
    if (col.a < .1) {
        discard;
    }
    FragColor = col;
}
