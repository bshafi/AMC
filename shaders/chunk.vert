#version 330

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 atexCoords;
layout (location = 2) in uint block_id;

// TODO: Change this to an ivec2 and add a uniform int for a height
uniform ivec3 chunk_pos;

// TODO: Move all the global variables into a uniform buffer
// width / height
uniform float aspect_ratio;

// TODO: Replace object_pos, object_rot, camera_pos, and camera_rot with a mat4x4 view and projection matrix

uniform vec3 camera_pos;
uniform vec3 camera_rot;

out vec2 texCoords;

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
    int local_y = gl_InstanceID / 256;
    int local_x = (gl_InstanceID % 256) % 16;
    int local_z = (gl_InstanceID % 256) / 16;
    ivec3 local_coordinates = ivec3(local_x, local_y, local_z);

    vec3 object_pos = 16 * chunk_pos + local_coordinates;
    vec3 object_rot = vec3(0.0f, 0.0f, 0.0f); 

    texCoords = atexCoords;

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