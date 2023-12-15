#version 430

// Input and output topologies
layout(triangles) in;
layout(line_strip, max_vertices = 170) out;

// Input
layout(location = 0) in vec2 v_texture_coord[];
layout(location = 1) in vec3 v_normal[];

// Uniform properties
uniform mat4 View;
uniform mat4 Projection;
uniform mat4 viewMatrices[6];

// TODO(student): Declare other uniforms here
uniform vec3 viewVector;

// Output
layout(location = 0) out vec2 texture_coord;


void EmitPoint(vec3 pos, vec3 offset)
{
    gl_Position = Projection * View * vec4(pos + offset, 1.0);
    EmitVertex();
}

vec3 calc_New_point(vec3 p1, vec3 p2, float dot1, float dot2)
{
    vec3 p12 = p1 + (p2 - p1) * (abs(dot1) / (abs(dot1) + abs(dot2)));
    return p12;
}

void emit_outline(int layer)
{
    vec3 p1 = gl_in[0].gl_Position.xyz;
    vec3 p2 = gl_in[1].gl_Position.xyz;
    vec3 p3 = gl_in[2].gl_Position.xyz;

    vec3 n1 = v_normal[0];
    vec3 n2 = v_normal[1];
    vec3 n3 = v_normal[2];

    float dotProduct1 = dot(n1, viewVector);
    float dotProduct2 = dot(n2, viewVector);
    float dotProduct3 = dot(n3, viewVector);
    float dots[3] = {dotProduct1, dotProduct2, dotProduct3};
    vec3 points[3] = {p1, p2, p3};

    if (dotProduct2 > 0)
    {
        if(dotProduct1 < 0) {
            vec3 p12 = calc_New_point(p1, p2, dotProduct1, dotProduct2);
            gl_Position = Projection * viewMatrices[layer] * vec4(p12, 1.0);
            EmitVertex();
        }

        if(dotProduct3 < 0) {
            vec3 p23 = calc_New_point(p2, p3, dotProduct2, dotProduct3);
            gl_Position = Projection * viewMatrices[layer] * vec4(p23, 1.0);
            EmitVertex();
        }
    }

    if (dotProduct1 > 0)
    {
        if(dotProduct3 < 0) {
            vec3 p12 = calc_New_point(p3, p1, dotProduct3, dotProduct1);
            gl_Position = Projection * viewMatrices[layer] * vec4(p12, 1.0);
            EmitVertex();
        }

        if(dotProduct2 < 0) {
            vec3 p23 = calc_New_point(p1, p2, dotProduct1, dotProduct2);
            gl_Position = Projection * viewMatrices[layer] * vec4(p23, 1.0);
            EmitVertex();
        }
    }

    if (dotProduct3 > 0)
    {
        if(dotProduct2 < 0) {
            vec3 p12 = calc_New_point(p2, p3, dotProduct2, dotProduct3);
            gl_Position = Projection * viewMatrices[layer] * vec4(p12, 1.0);
            EmitVertex();
        }

        if(dotProduct1 < 0) {
            vec3 p23 = calc_New_point(p3, p1, dotProduct3, dotProduct1);
            gl_Position = Projection * viewMatrices[layer] * vec4(p23, 1.0);
            EmitVertex();
        }
    }
    EndPrimitive();
}
void main()
{

    for(int i = 0; i < 6; i++)
    {
        gl_Layer = i;
        emit_outline(i);
    }
}

