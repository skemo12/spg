#version 430

// Input and output topologies
layout(triangles) in;
layout(triangle_strip, max_vertices = 170) out;

// Input
layout(location = 0) in vec2 v_texture_coord[];

// Uniform properties
uniform mat4 View;
uniform mat4 Projection;
uniform int instances;
// TODO(student): Declare other uniforms here
uniform float shrink;

// Output
layout(location = 0) out vec2 texture_coord;


void EmitPoint(vec3 pos, vec3 offset)
{
    gl_Position = Projection * View * vec4(pos + offset, 1.0);
    EmitVertex();
}


void main()
{
    vec3 p1 = gl_in[0].gl_Position.xyz;
    vec3 p2 = gl_in[1].gl_Position.xyz;
    vec3 p3 = gl_in[2].gl_Position.xyz;
    vec3 center = (p1 + p2 + p3) / 3;
    const vec3 INSTANCE_OFFSET = vec3(1.25, 0, 1.25);
    const int NR_COLS = 6;
    p1 = center + (p1 - center) * shrink;
    p2 = center + (p2 - center) * shrink;
    p3 = center + (p3 - center) * shrink;
    // TODO(student): Second, modify the points so that the
    // triangle shrinks relative to its center
    int val = 0;
    float zoff = 0;
    for (int i = 0; i <= instances; i++)
    {
        // TODO(student): First, modify the offset so that instances
        // are displayed on `NR_COLS` columns. Test your code by
        // changing the value of `NR_COLS`. No need to recompile.
        vec3 offset = vec3(i % 4 * 1.25, 0, zoff);
        val++;
        if (val == 4) {
            val = 0;
            zoff += 1.25;
        }

        texture_coord = v_texture_coord[0];
        EmitPoint(p1, offset);

        texture_coord = v_texture_coord[1];
        EmitPoint(p2, offset);

        texture_coord = v_texture_coord[2];
        EmitPoint(p3, offset);

        EndPrimitive();
    }
}
