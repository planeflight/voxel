#shader vertex
#version 450

layout(location=0) in int a_data;
       
layout(location=0) out vec3 v_pos;
layout(location=1) out vec3 v_chunk_pos;
layout(location=2) out vec2 v_tex_coords;
layout(location=3) out vec3 v_normal;

uniform mat4 u_projection;
uniform mat4 u_view;

uniform vec3 u_chunk_offset;
uniform vec3 u_chunk_size;

void main() {
    // compute world offset
    vec3 pos = vec3(
        float((a_data & 0xF) >> 0),
        float((a_data & 0x7F0) >> 4),
        float((a_data & 0x7800) >> 11)
    );
    v_chunk_pos = pos;
    pos += u_chunk_offset * u_chunk_size;

    gl_Position = u_projection * u_view * vec4(pos, 1.0);

    // set varyings
    v_pos = pos;
    v_tex_coords = vec2(
        float((a_data & 0x1C0000) >> 18),
        float((a_data & 0xE00000) >> 21)
    );
    v_tex_coords /= 4.0;

    // compute normal
    int normal_idx = int(
        (a_data & 0x38000) >> 15
    );
    if (normal_idx == 0) {
        // left
        v_normal = vec3(-1.0, 0.0, 0.0);
    } else if (normal_idx == 1) {
        // right
        v_normal = vec3(1.0, 0.0, 0.0);
    } else if (normal_idx == 2) {
        // forward
        v_normal = vec3(0.0, 0.0, 1.0);
    } else if (normal_idx == 3) {
        // backward
        v_normal = vec3(0.0, 0.0, -1.0);
    } else if (normal_idx == 4) {
        // top
        v_normal = vec3(0.0, 1.0, 0.0);
    } else if (normal_idx == 5) {
        // bottom
        v_normal = vec3(0.0, -1.0, 0.0);
    }
}

#shader fragment
#version 450
layout(location=0) in vec3 v_pos;
layout(location=1) in vec3 v_chunk_pos;
layout(location=2) in vec2 v_tex_coords;
layout(location=3) in vec3 v_normal;

out vec4 color;

uniform sampler2D u_texture;

vec3 hack_lighting(vec3 original) {
    vec3 co = original;
    if (v_normal.y == 1.0) {
        co *= 1.0;
    } else if (v_normal.x != 0.0) {
        co *= 0.8;
    } else if (v_normal.z != 0.0) {
        co *= 0.6;
    } else {
        co *= 0.5;
    }
    return co;
}

void main() {
    vec4 tex_color = texture(u_texture, v_tex_coords);
    color.a = tex_color.a;
    color.rgb = hack_lighting(tex_color.rgb);
}
