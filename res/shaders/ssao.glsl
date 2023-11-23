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

void main() {
    float radius = 0.35;
    float bias = 0.0025;
    int kernel_size = 16;

    vec2 noise_scale = vec2(1600.0, 900.0) / 4.0;

    vec3 frag_pos = texture(u_position, v_tex_coords).xyz;
    frag_pos = (u_view * vec4(frag_pos, 1.0)).xyz;
    vec3 normal = texture(u_normal, v_tex_coords).xyz;
    normal = (u_view * vec4(normal, 1.0)).xyz;

    vec3 random_vec = normalize(texture(u_noise, v_tex_coords * noise_scale).xyz);

    vec3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;

    for (int i = 0; i < kernel_size; ++i) {
        // get sample position
        vec3 sample_pos = tbn * u_ssao_samples[i];
        sample_pos = frag_pos + sample_pos * radius;

        // project sample position onto clip space and then to sample texture
        vec4 offset = vec4(sample_pos, 1.0);
        offset = u_projection * offset;
        offset.xy /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;

        float sample_depth = (
            u_view * vec4(texture(u_position, offset.xy).xyz, 1.0)
        ).z;

        // range check & accumulate
        float range_check = smoothstep(0.0, 1.0, radius / abs(frag_pos.z - sample_depth));
        occlusion += (sample_depth >= sample_pos.z + bias ? 1.0 : 0.0) * range_check;

    }
    occlusion = 1.0 - (occlusion / kernel_size);
    occlusion = clamp(occlusion, 0.0, 1.0);
    occlusion = pow(occlusion, 2.0);

    color = vec4(vec3(occlusion), 1.0);
    // color = vec4(1.0, 0.0, 0.0, 1.0);
}
