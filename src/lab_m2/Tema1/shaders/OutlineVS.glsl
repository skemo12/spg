#version 430

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;

// Output
layout(location = 0) out vec2 texture_coord;
layout(location = 1) out vec3 normal;


void main()
{
    texture_coord = v_texture_coord;
    normal = v_normal;
    gl_Position = Model * vec4(v_position, 1);
}
