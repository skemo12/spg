#version 430

// Input
layout(location = 0) in vec3 world_position;
layout(location = 1) in vec3 world_normal;

// Uniform properties
uniform samplerCube texture_cubemap;
uniform int type;

uniform vec3 camera_position;

// Output
layout(location = 0) out vec4 out_color;


vec3 myReflect()
{
    // TODO(student): Compute the reflection color value
    return texture(texture_cubemap, reflect(world_position - camera_position, normalize(world_normal))).xyz;
    //return vec3(0.5);

}


vec3 myRefract(float refractive_index)
{
    // TODO(student): Compute the refraction color value
    return texture(texture_cubemap, refract(world_position - camera_position, normalize(world_normal), 1.0f / refractive_index)).xyz;
    //return vec3(0.5);

}


void main()
{

    if (type == 0)
    {
        out_color = vec4(myReflect(), 0);
    }
    else
    {
        out_color = vec4(myRefract(1.33), 0);
    }
}
