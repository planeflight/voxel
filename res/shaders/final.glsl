#shader vertex

layout(location=0) in vec2 a_position;
layout(location=1) in vec2 a_tex_coords;

out vec2 v_tex_coords;

void main() {
    gl_Position = vec4(a_position, 0.0, 1.0);
    v_tex_coords = a_tex_coords;
}

#shader fragment

in vec2 v_tex_coords;

uniform sampler2D color;
uniform sampler2D normal;
uniform sampler2D depth;

void main() {
    float d = texture2D(depth, v_tex_coords).r;

    
}
