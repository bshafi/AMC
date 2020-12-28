#version 330

layout (location = 0) in vec4 pos;

// width / height
uniform float aspect_ratio;

uniform vec3 object_pos;
uniform vec3 object_rot;

uniform vec3 camera_pos;
uniform vec3 camera_rot;

#define M_PI 3.1415926535897932384626433832795



mat4x4 rotate(vec3 rotation) {
    float a = rotation.z;
    float b = rotation.y;
    float c = rotation.x;


    return transpose(mat4x4(
        cos(a) * cos(b), cos(a) * sin(b) * sin(c) - sin(a) * cos(c), cos(a) * sin(b) * cos(c) + sin(a) * sin(c), 0.0f,
        sin(a) * cos(b), sin(a) * sin(b) * sin(c) + cos(a) * cos(c), sin(a) * sin(b) * cos(c) - cos(a) * sin(c), 0.0f,
                -sin(b),                            cos(b) * sin(c),                            cos(b) * cos(c), 0.0f,
                   0.0f,                                       0.0f,                                       0.0f, 1.0f
    ));
}
mat4x4 translate(vec3 offset) {
    return transpose(mat4x4(
        1.0f, 0.0f, 0.0f, offset.x,
        0.0f, 1.0f, 0.0f, offset.y,
        0.0f, 0.0f, 1.0f, offset.z,
        0.0f, 0.0f, 0.0f,     1.0f
    ));
}

void main() {
    float far = 100.0f;
    float near = 0.1f;
    float FOV = M_PI / 2;
    float skew = 1/tan(FOV/2);
    mat4x4 projection = mat4x4(
        skew, 0, 0, 0,
        0, skew * aspect_ratio, 0, 0,
        0, 0, -2/(far - near), -(far + near)/(far - near),
        0, 0, 0, 1
    );
    
    mat4x4 model = translate(object_pos) * rotate(object_rot);
    mat4x4 view = rotate(camera_rot) * translate(camera_pos);
    gl_Position = projection * view * model * vec4(pos.xyz, 1.0f);
}