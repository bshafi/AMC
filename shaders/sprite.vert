#version 410

layout (location = 0) in uint pos;

out vec2 texture_coordinates;

uniform uvec2 texture_bounds;
uniform uvec2 window_bounds;

uniform vec4 source_rect;
uniform vec4 dest_rect;
uniform uint depth;

vec2 vertices[] = vec2[](
    vec2(0, 1),
    vec2(1, 0),
    vec2(0, 0),
    vec2(1, 1)
);

mat4 ortho(float left, float right, float bottom, float top, float near, float far) {
    return mat4(
        2 / (right - left), 0, 0, 0,
        0, 2 / (top - bottom), 0, 0,
        0, 0, -2 / (far - near), 0,
        -(right + left)/ (right - left), -(top + bottom) / (top - bottom), -(far + near)/ (far - near), 1
    );
}

void main() {
    texture_coordinates = (vertices[pos] * source_rect.zw + source_rect.xy) / texture_bounds;
    gl_Position = ortho(0, window_bounds.x, window_bounds.y, 0, -1000, 10000) * vec4(vertices[pos] * dest_rect.zw + dest_rect.xy, depth, 1);
}