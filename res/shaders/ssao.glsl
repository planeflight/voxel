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
    float radius = 0.02;
    float bias = 0.0025;
    int kernel_size = 8;

    vec3 tangent = normalize(rot - normal * dot(rot, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn = mat3(tangent, bitangent, normal);

    float ao = 0.0;
    for (int i = 0; i < kernel_size; ++i) {
        // get sample position
        vec3 sample_pos = pos + tbn * u_ssao_samples[int(rand(rot.xy)) * 64] * radius;

        vec4 offset = u_projection * vec4(sample_pos, 1.0);
        offset.xy /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;

        // get actual depth
        float d = (
            u_view * vec4(texture(u_position, offset.xy).xyz, 1.0)
        ).z;

        float range_check = smoothstep(0.0, 1.0, radius / abs(pos.z - d));
        ao += (d >= sample_pos.z + bias ? 1.0 : 0.0) * range_check;
    }
    ao /= float(kernel_size);
    ao = pow(ao, 2.2);
    ao = 1.0 - ao;
    ao = clamp(ao, 0.0, 1.0);
    return ao;
}

void main() {
    vec3 pos = texture(u_position, v_tex_coords).xyz;
    pos = (u_view * vec4(pos, 1.0)).xyz;
    vec3 n = texture(u_normal, v_tex_coords).xyz;
    n = (u_view * vec4(n, 1.0)).xyz;
    vec3 r = texture(u_noise, vec2(rand(pos.xy), rand(pos.yz))).rgb;

    float ao = ssao(pos, n, r);
    color = vec4(vec3(ao), 1.0);
    // color = vec4(1.0, 0.0, 0.0, 1.0);
}
