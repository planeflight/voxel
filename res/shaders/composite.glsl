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

struct DirectionLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 u_view_pos;
uniform DirectionLight u_sunlight;

const int POISSON_SAMPLES = 16;
vec2 poisson_disk[POISSON_SAMPLES] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

float get_shadow() {
    vec4 frag_pos_light_space = u_light_space * texture(u_position, v_tex_coords);
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    // convert to [0, 1]
    proj_coords = proj_coords * 0.5 + vec3(0.5);

    // closest depth to light
    float closest_depth = texture(u_depth_map, proj_coords.xy).x;
    // fragment depth
    float frag_depth = proj_coords.z;
    float bias = 0.002;
    // float shadow = frag_depth - bias > closest_depth ? 1.0 : 0.0;
    float shadow = 0.0;
    vec2 texel_size = vec2(1.0) / textureSize(u_depth_map, 0);
    for (int i = 0; i < POISSON_SAMPLES; ++i) {
        float d = texture(u_depth_map, proj_coords.xy + poisson_disk[i] * texel_size).x;
        shadow += (frag_depth - bias > d) ? 1.0 : 0.0;
    }

    return shadow / POISSON_SAMPLES;
}

vec3 get_sunlight() {
    vec3 position = texture(u_position, v_tex_coords).rgb;
    vec3 normal = texture(u_normal, v_tex_coords).rgb;

    vec3 view_dir = normalize(u_view_pos - position);

    // ambient
    vec3 ambient = u_sunlight.ambient;
    // diffuse
    vec3 light_dir = normalize(u_sunlight.direction);
    vec3 diffuse = max(dot(normal, light_dir), 0.0) * u_sunlight.diffuse;
    // specular
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = spec * u_sunlight.specular;

    vec3 result = ambient + (1.0 - get_shadow()) * (diffuse + specular);
    return result;
}

void main() {
    vec3 object_color = texture(u_color, v_tex_coords).rgb;

    // ambient, diffuse from sun
    color = vec4(get_sunlight() * object_color, 1.0);
}
