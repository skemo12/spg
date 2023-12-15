#version 430

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;
// TODO (student): Add the new attributes neccessary for skinning
layout(location = 3) in ivec4 BoneIDs;
layout(location = 4) in vec4 Weights;

const int MAX_BONES = 200;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
// TODO (student): Declare a new uniform variable array, which will
// receive the bone transformations
uniform mat4 Bones[MAX_BONES];


// Output
layout(location = 0) out vec2 texture_coord;
// TODO (student): Send the normal to the fragment shader
layout(location = 1) out vec3 normal;

void main()
{
    // TODO (student): Compute the final bone transformation
    mat4 BoneTransform = Bones[BoneIDs[0]] * Weights[0];
    BoneTransform += Bones[BoneIDs[1]] * Weights[1];
    BoneTransform += Bones[BoneIDs[2]] * Weights[2];
    BoneTransform += Bones[BoneIDs[3]] * Weights[3];
    texture_coord = v_texture_coord;

    // TODO (student): Compute the normal
    normal = mat3(BoneTransform) * v_normal; 

    // TODO (student): Apply the bone transformation on the vertex position
    gl_Position = Projection * View * Model * BoneTransform * vec4(v_position, 1.0);
}
