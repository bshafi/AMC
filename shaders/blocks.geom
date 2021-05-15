#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 36)  out;

// reference: https://stackoverflow.com/questions/28375338/cube-using-single-gl-triangle-strip

flat in uint h_block_id[];

out vec3 cube_tex_coords;

layout (std140) uniform globals_3d {
    mat4x4 view;
    mat4x4 projection;
};

uniform ivec2 chunk_pos;
vec2[] texture_coordinates = vec2[](
    vec2(0.f / 6, 0.0f),
    vec2(0.f / 6, 1.0f),
    vec2(1.f / 6, 1.0f),

    vec2(0.f / 6, 0.0f),
    vec2(1.f / 6, 1.0f),
    vec2(1.f / 6, 0.0f),

    vec2(1.f / 6, 0.0f),
    vec2(1.f / 6, 1.0f),
    vec2(2.f / 6, 1.0f),

    vec2(1.f / 6, 0.0f),
    vec2(2.f / 6, 1.0f),
    vec2(2.f / 6, 0.0f),

    vec2(2.f / 6, 0.0f),
    vec2(2.f / 6, 1.0f),
    vec2(3.f / 6, 1.0f),

    vec2(2.f / 6, 0.0f),
    vec2(3.f / 6, 1.0f),
    vec2(3.f / 6, 0.0f),

    vec2(3.f / 6, 0.0f),
    vec2(3.f / 6, 1.0f),
    vec2(4.f / 6, 1.0f),

    vec2(3.f / 6, 0.0f),
    vec2(4.f / 6, 1.0f),
    vec2(4.f / 6, 0.0f),

    vec2(4.f / 6, 0.0f),
    vec2(4.f / 6, 1.0f),
    vec2(5.f / 6, 1.0f),

    vec2(4.f / 6, 0.0f),
    vec2(5.f / 6, 1.0f),
    vec2(5.f / 6, 0.0f),

    vec2(5.f / 6, 0.0f),
    vec2(5.f / 6, 1.0f),
    vec2(6.f / 6, 1.0f),

    vec2(5.f / 6, 0.0f),
    vec2(6.f / 6, 1.0f),
    vec2(6.f / 6, 0.0f)
);

vec3[] vertices = vec3[](
    vec3(-0.5f,  0.5f,  0.5f),
    vec3(-0.5f,  0.5f, -0.5f),
    vec3( 0.5f,  0.5f, -0.5f),

    vec3(-0.5f,  0.5f,  0.5f),
    vec3( 0.5f,  0.5f, -0.5f),
    vec3( 0.5f,  0.5f,  0.5f),

    vec3(-0.5f, -0.5f,  0.5f),
    vec3(-0.5f, -0.5f, -0.5f),
    vec3( 0.5f, -0.5f, -0.5f),

    vec3(-0.5f, -0.5f,  0.5f),
    vec3( 0.5f, -0.5f, -0.5f),
    vec3( 0.5f, -0.5f,  0.5f),

    vec3(-0.5f, -0.5f,  0.5f),
    vec3(-0.5f,  0.5f,  0.5f),
    vec3( 0.5f,  0.5f,  0.5f),

    vec3(-0.5f, -0.5f,  0.5f),
    vec3( 0.5f,  0.5f,  0.5f),
    vec3( 0.5f, -0.5f,  0.5f),

    vec3(-0.5f, -0.5f, -0.5f),
    vec3(-0.5f,  0.5f, -0.5f),
    vec3( 0.5f,  0.5f, -0.5f),
    
    vec3(-0.5f, -0.5f, -0.5f),
    vec3( 0.5f,  0.5f, -0.5f),
    vec3( 0.5f, -0.5f, -0.5f),

    
    vec3( 0.5f, -0.5f,  0.5f),
    vec3( 0.5f,  0.5f,  0.5f),
    vec3( 0.5f,  0.5f, -0.5f),

    vec3( 0.5f, -0.5f,  0.5f),
    vec3( 0.5f,  0.5f, -0.5f),
    vec3( 0.5f, -0.5f, -0.5f),

    vec3(-0.5f, -0.5f, -0.5f),
    vec3(-0.5f,  0.5f, -0.5f),
    vec3(-0.5f,  0.5f,  0.5f),

    vec3(-0.5f, -0.5f, -0.5f),
    vec3(-0.5f,  0.5f,  0.5f),
    vec3(-0.5f, -0.5f,  0.5f)
);

flat out uint block_id;
out vec2 tex_coords;

void main() {
    block_id = h_block_id[0];
    vec3 center = vec3(float(chunk_pos.x * 16), 0, float(chunk_pos.y * 16)) + gl_in[0].gl_Position.xyz + vec3(0.5, 0.5, 0.5);

    for (int i = 0; i < 36; i = i += 3) {
        tex_coords = texture_coordinates[i + 0];
        block_id = h_block_id[0];
        gl_Position = projection * view * vec4(center + vertices[i + 0], 1.0);
        EmitVertex();

        tex_coords = texture_coordinates[i + 1];
        block_id = h_block_id[0];
        gl_Position = projection * view * vec4(center + vertices[i + 1], 1.0);
        EmitVertex();

        tex_coords = texture_coordinates[i + 1];
        block_id = h_block_id[0];
        gl_Position = projection * view * vec4(center + vertices[i + 1], 1.0);
        EmitVertex();

        EndPrimitive();
    }
}