#version 430

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform float deltaTime;
out float vert_lifetime;
out float vert_iLifetime;

struct Particle
{
    vec4 position;
    vec4 speed;
    vec4 iposition;
    vec4 ispeed;
    float delay;
    float iDelay;
    float lifetime;
    float iLifetime;
};


struct BezierLineData {
    vec3 c0;
    vec3 c1;
    vec3 c2;
    vec3 c3;
};
uniform BezierLineData bezierLines[5];
layout(std430, binding = 0) buffer particles {
    Particle data[];
};





vec3 BezierCurve(float t, vec3 points[4]) {
    float u = 1 - t;
    float tt = t*t;
    float uu = u*u;
    float uuu = uu * u;
    float ttt = tt * t;

    vec3 B = uuu * points[0]; //influence of P0
    B += 3 * uu * t * points[1]; //influence of P1
    B += 3 * u * tt * points[2]; //influence of P2
    B += ttt * points[3]; //influence of P3

    return B;
}

float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 bezier_points[5][4];


void main()
{
    bezier_points[0][0] = vec3(0, 0, 0);
    bezier_points[0][1] = vec3(2, 2, 0);
    bezier_points[0][2] = vec3(0.5, 0.2, 0);
    bezier_points[0][3] = vec3(2.5, 2.5, 0);

    bezier_points[1][0] = vec3(0, 0, 0);
    bezier_points[1][1] = vec3(1.2, 1.2, 1); // Adjusted z-coordinate
    bezier_points[1][2] = vec3(3.2, 0.6, 2);   // Adjusted z-coordinate
    bezier_points[1][3] = vec3(0.2, 1.8, 1); // Adjusted z-coordinate

    bezier_points[2][0] = vec3(0, 0, 0);
    bezier_points[2][1] = vec3(-0.8, 1.5, -2);   // Adjusted z-coordinate
    bezier_points[2][2] = vec3(-0.1, 0.33, -1);   // Adjusted z-coordinate
    bezier_points[2][3] = vec3(-2.2, 3.1, -4); // Adjusted z-coordinate

    bezier_points[3][0] = vec3(0, 0, 0);
    bezier_points[3][1] = vec3(-1.4, 0.4, -1);   // Adjusted z-coordinate
    bezier_points[3][2] = vec3(-3.4, 1.4, -1);   // Adjusted z-coordinate
    bezier_points[3][3] = vec3(-1.4, 3.4, -1); // Adjusted z-coordinate

    bezier_points[4][0] = vec3(0, 0, 0);
    bezier_points[4][1] = vec3(-1.5, 1.5, 0);   // Adjusted z-coordinate
    bezier_points[4][2] = vec3(-2.5, 0.4, 0);   // Adjusted z-coordinate
    bezier_points[4][3] = vec3(-4.5, -1, 0); // Adjusted z-coordinate



    float dt = deltaTime;
    vec3 spd = data[gl_VertexID].speed.xyz;
    vec3 pos = data[gl_VertexID].position.xyz;

    float t = data[gl_VertexID].lifetime / data[gl_VertexID].iLifetime; // normalize lifetime to [0, 1]
    vec3 bezier = BezierCurve(1.0 - t, bezier_points[gl_VertexID % 5]);
    pos = bezier / 2;
    data[gl_VertexID].lifetime -= deltaTime;
    data[gl_VertexID].delay -= deltaTime;
    if (data[gl_VertexID].lifetime < 0.0) {
        data[gl_VertexID].lifetime = data[gl_VertexID].iLifetime;
        data[gl_VertexID].position = data[gl_VertexID].iposition;
        data[gl_VertexID].speed = data[gl_VertexID].ispeed;
        data[gl_VertexID].delay = data[gl_VertexID].iDelay;
    }

    // spd = spd + vec3(0.1, 0.1, 0.1) * dt;
    if (data[gl_VertexID].delay > 0.0) {
        data[gl_VertexID].delay -= deltaTime;
        pos = data[gl_VertexID].iposition.xyz;
        spd = data[gl_VertexID].ispeed.xyz;
        data[gl_VertexID].lifetime = data[gl_VertexID].iLifetime;

    }
    // // Update position and speed to make particles rise in a line
    // pos = pos + spd * dt;
    // spd = spd + vec3(0, 1.0, 0) * dt; // Increase the y-component of speed to make particles rise

    // // Reset particle to initial position and speed when it reaches a certain height
    // if(pos.y > (40 + rand(pos.xy) * 20))
    // {
    //     pos = data[gl_VertexID].iposition.xyz;
    //     spd = data[gl_VertexID].ispeed.xyz;
    // }

    data[gl_VertexID].position.xyz =  pos;
    data[gl_VertexID].speed.xyz =  spd;
    // data[gl_VertexID].speed.xyz =  spd;

    gl_Position = Model * vec4(pos, 1);
}
