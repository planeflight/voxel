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

uniform sampler2D u_position;
uniform sampler2D u_normal;
uniform sampler2D u_color;

uniform sampler2D u_depth_map;
uniform mat4 u_light_space;

out vec4 color;

uniform vec3 u_view_pos;

vec3 get_diffuse(vec3 light_position, vec3 light_color) {
    vec3 position = texture(u_position, v_tex_coords).rgb;
    vec3 normal = texture(u_normal, v_tex_coords).rgb;

    vec3 light_dir = normalize(light_position - position);
    vec3 diffuse = max(dot(normal, light_dir), 0.0) * light_color;
    return diffuse;
}

float get_shadow() {
    vec4 frag_pos_light_space = u_light_space * texture(u_position, v_tex_coords);
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    // convert to [0, 1]
    proj_coords = proj_coords * 0.5 + vec3(0.5);

    // closest depth to light
    float closest_depth = texture(u_depth_map, proj_coords.xy).x;
    // fragment depth
    float frag_depth = proj_coords.z;
    float bias = 0.005;
    float shadow = frag_depth - bias > closest_depth ? 1.0 : 0.0;
    return shadow;
}

void main() {
    vec3 object_color = texture(u_color, v_tex_coords).rgb;

    // ambient from sun
    vec3 light_color = vec3(1.0, 1.0, 1.0);

    float ambient_strength = 0.18;
    vec3 ambient = ambient_strength * light_color;
    vec3 diffuse = get_diffuse(vec3(10.0), light_color);

    vec3 result = (ambient + (1.0 - get_shadow()) * diffuse) * object_color;

    color = vec4(result, 1.0);
}
