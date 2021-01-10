#version 330 core

out vec4 FragColor;

uniform vec3 outColor;

in vec2 texCoords;
uniform sampler2D tex;

void main() {
    FragColor = texture(tex, texCoords);
    //vec4 f = texture(tex, texCoords);
    //FragColor = vec4(texCoords.x, texCoords.y, 0.00000001f * f.x, 1.0f);
}