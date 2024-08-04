#shader vertex
#version 450

layout(location=0) in int a_data;
       
layout(location=0) out vec3 v_pos;
layout(location=1) out vec2 v_tex_coords;
layout(location=2) out vec3 v_normal;

uniform mat4 u_projection;
uniform mat4 u_view;

uniform vec3 u_chunk_offset;
uniform vec3 u_chunk_size;

void main() {
    // compute world offset
    vec3 pos = vec3(
        float((a_data & 0xF) >> 0),
        float((a_data & 0xFF0) >> 4),
        float((a_data & 0xF000) >> 12)
    );
    pos += u_chunk_offset * u_chunk_size;
    v_pos = pos;

    gl_Position = u_projection * u_view * vec4(pos, 1.0);

    // set varyings
    v_tex_coords = vec2(
        float((a_data & 0x380000) >> 19),
        float((a_data & 0x1C00000) >> 22)
    );
    v_tex_coords /= 4.0;

    // compute normal
    int normal_idx = int(
        (a_data & 0x70000) >> 16
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
layout(location=1) in vec2 v_tex_coords;
layout(location=2) in vec3 v_normal;

layout(location=0) out vec4 position;
layout(location=1) out vec3 normal;
layout(location=2) out vec4 color;

uniform sampler2D u_texture;

void main() {
    vec4 tex_color = texture(u_texture, v_tex_coords);

    // MAKE w=1.0 OR ELSE SHADOWS DON'T WORK
    position = vec4(v_pos, 1.0);
    normal = v_normal;
    color.rgb = tex_color.rgb;
    color.a = 1.0;
}
