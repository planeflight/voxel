#shader vertex
#version 450

layout(location=0) in vec2 a_pos;

uniform float u_height;
uniform mat4 u_view_proj;
uniform float u_time;
uniform vec2 u_cam_pos;

uniform sampler2D u_noise;

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = rand(i);
    float b = rand(i + vec2(1.0, 0.0));
    float c = rand(i + vec2(0.0, 1.0));
    float d = rand(i + vec2(1.0, 1.0));

    // Smooth Interpolation

    // Cubic Hermine Curve. Same as SmoothStep()
    vec2 u = f*f*(3.0-2.0*f);
    // u = smoothstep(0.,1.,f);

    // Mix 4 corners percentages
    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

float fbm(vec2 st, int octaves, float amp) {
    float t = 0.;
    float a = 1., f = 1.;
    for (int i = 0; i < octaves; ++i) {
        t += a * noise(st * f);
        f *= 2.;
        a *= amp;
    }
    return t;
}

void main() {
    vec3 pos = vec3(a_pos.x, u_height, a_pos.y);
    pos.xz += round(u_cam_pos);

    // calculate height offset
    float h = 0.9 * noise(pos.xz * 0.05 + 0.5 * sin(u_time));
    h += 0.9 * fbm(pos.xz * 0.2 + 0.1 * u_time, 4, 0.5);
    h += 0.8 * sin(0.1 * u_time);
    pos.y += h;
    // set gl pos
    gl_Position = vec4(pos, 1.); // no projection because we need nonprojected to calculate normals
}

#shader geometry
#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout(location=0) out vec3 v_pos;
layout(location=1) out vec3 v_normal;

// camera uniforms
uniform mat4 u_view;
uniform mat4 u_projection;

vec3 calc_normal(vec3 v0, vec3 v1, vec3 v2) {
    vec3 t0 = v1 - v0;
    vec3 t1 = v2 - v0;
    return normalize(cross(t0, t1));
}

void main() {
    vec3 normal = normalize(calc_normal(gl_in[0].gl_Position.xyz,
                                        gl_in[1].gl_Position.xyz,
                                        gl_in[2].gl_Position.xyz));
    for (int i = 0; i < 3; ++i) {
        gl_Position = u_projection * u_view * gl_in[i].gl_Position;
        v_pos = gl_in[i].gl_Position.xyz;
        v_normal = normal;
        EmitVertex();
    }
    EndPrimitive();
}

#shader fragment
#version 450

layout(location=0) in vec3 v_pos;
layout(location=1) in vec3 v_normal;

layout(location=0) out vec4 position;
layout(location=1) out vec3 normal;
layout(location=2) out vec3 color;

void main() {
    position = vec4(v_pos, 1.);
    normal = v_normal;
    color = vec3(0.8, 0.8, 1.);
}
