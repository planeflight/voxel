#shader vertex
#version 450

layout(location=0) in vec2 a_position;

layout(location=0) out vec2 v_tex_coords;

void main() {
    gl_Position = vec4(a_position, 0.0, 1.0);
    v_tex_coords = a_position / 2.0 + vec2(0.5);
}

#shader fragment
#version 450

layout(location=0) in vec2 v_tex_coords;

out vec4 color;

uniform sampler2D u_position;
uniform sampler2D u_normal;

uniform sampler2D u_noise;
uniform vec3[64] u_ssao_samples;

uniform mat4 u_projection;
uniform mat4 u_view;

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float ssao(vec3 pos, vec3 normal, vec3 rot) {
    float radius = 0.5;
    float bias = 0.03;
    int kernel_size = 32;

    vec3 tangent = normalize(rot - normal * dot(rot, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn = mat3(tangent, bitangent, normal);

    float ao = 0.0;
    for (int i = 0; i < kernel_size; ++i) {
        // get sample position
        vec3 sample_pos = pos + tbn * u_ssao_samples[i] * radius;

        vec4 offset = u_projection * vec4(sample_pos, 1.0);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        // sample the position buffer at this screen space position
        vec3 sample_pos_world = texture(u_position, offset.xy).xyz;
        vec3 sample_pos_view = vec3(u_view * vec4(sample_pos_world, 1.0)); // Transform to view space
        // get actual depth
        float d = sample_pos_view.z;

        float range_check = smoothstep(0.0, 1.0, radius / length(pos - sample_pos_view));
        ao += (d >= sample_pos.z + bias ? 1.0 : 0.0) * range_check;
    }
    ao /= float(kernel_size);
    ao = 1.0 - ao;
    ao = pow(ao, 2.2);
    ao = clamp(ao, 0.0, 1.0);
    return ao;
}

void main() {
    vec3 pos = texture(u_position, v_tex_coords).xyz;
    pos = (u_view * vec4(pos, 1.0)).xyz;
    vec3 n = texture(u_normal, v_tex_coords).xyz;
    n = mat3(u_view) * n;
    n = normalize(n);
    vec3 r = texture(u_noise, v_tex_coords * vec2(1600., 900.) / 4.).xyz;
    r = normalize(r);
    r = r * 2. - 1.; // convert to [-1, 1]

    float ao = ssao(pos, n, r);
    color = vec4(vec3(ao), 1.0);
}
