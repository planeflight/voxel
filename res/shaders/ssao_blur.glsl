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

out float color;

uniform sampler2D u_ssao;

void main() {
    vec2 texel_size = 1.0 / vec2(textureSize(u_ssao, 0));
    float result = 0.0;
    for (int y = -2; y < 2; ++y) {
        for (int x = -2; x < 2; ++x) {
            vec2 offset = vec2(float(x), float(y)) * texel_size;
            result += texture(u_ssao, v_tex_coords + offset).r;
        }
    }
    color = result / (4.0 * 4.0);
}
