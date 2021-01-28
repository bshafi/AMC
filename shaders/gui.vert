#version 410

layout (location = 0) in vec2 pos;
layout (location = 1) in vec4 dest_rect;
layout (location = 2) in uint texture_id;
layout (location = 3) in vec4 source_rect;
layout (location = 4) in uint depth;


out vec2 texture_coordinates;
flat out uint tex_id;

bool vec2_equals(vec2 x, vec2 y) {
    return length(x - y) < 0.001f;
}



void main() {
    tex_id = texture_id;

    // TODO: Make this a uniform
    // Opengl clips any vertices that are outside of the [-1,1] range
    // Dividing the depth uint by this gives a float that is within range
    float MAX_IMAGE_LAYERS = 1000;

    if (vec2_equals(pos, vec2(0, 0))) {
        gl_Position = vec4(dest_rect.x, dest_rect.y, float(depth) / MAX_IMAGE_LAYERS, 1);
        texture_coordinates = vec2(source_rect.x, source_rect.y);
    } else if (vec2_equals(pos, vec2(0, 1))) {
        gl_Position = vec4(dest_rect.x, dest_rect.y + dest_rect.w, float(depth) / MAX_IMAGE_LAYERS, 1);
        texture_coordinates = vec2(source_rect.x, source_rect.y + source_rect.w);
    } else if (vec2_equals(pos, vec2(1, 1))) {
        gl_Position = vec4(dest_rect.x + dest_rect.z, dest_rect.y + dest_rect.w, float(depth) / MAX_IMAGE_LAYERS, 1);
        texture_coordinates = vec2(source_rect.x + source_rect.z, source_rect.y + source_rect.w);
    } else if (vec2_equals(pos, vec2(1, 0))) {
        gl_Position = vec4(dest_rect.x + dest_rect.z, dest_rect.y, float(depth) / MAX_IMAGE_LAYERS, 1);
        texture_coordinates = vec2(source_rect.x + source_rect.z, source_rect.y);
    } else {
        //gl_Position = vec4(0, 0, 0, 1.0f);
        //texture_coordinates = vec2(source_rect.x + source_rect.z, source_rect.y + source_rect.w);
    }
}