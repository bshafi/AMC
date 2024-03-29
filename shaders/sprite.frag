#version 410 core

out vec4 FragColor;

in vec2 texture_coordinates;

uniform sampler2D tex;


uniform vec4 color;
uniform float opacity;
uniform float color_mix;

void main() {
    vec4 col = texture(tex, texture_coordinates);

    FragColor = vec4(1, 1, 1, opacity) * mix(col, color, color_mix);
}
