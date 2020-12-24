#version 330

layout (location = 0) in vec4 pos;

uniform float rotation_around_y_axis;

#define M_PI 3.1415926535897932384626433832795

void main() {
    float far = 100.0f;
    float near = 0.1f;
    mat4x4 rot = mat4x4(
        cos(rotation_around_y_axis), 0, -sin(rotation_around_y_axis), 0,
                                  0, 1,                            0, 0,
        sin(rotation_around_y_axis), 0,  cos(rotation_around_y_axis), 0,
                                  0, 0,                            0, 1.0f
    );
    float S = 1 / tan(M_PI/4);
    mat4x4 perspective = mat4x4(
        S, 0,                          0,  0,
        0, S,                          0,  0,
        0, 0,          -far/(far - near), -1,
        0, 0, -(far * near)/(far - near),  0
    );
    gl_Position = perspective * (rot * pos + vec4(0, 0, -2, 0));
}