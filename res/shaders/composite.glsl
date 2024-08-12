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

uniform sampler2D u_ssao;

out vec4 color;

struct DirectionLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 u_view_pos;
uniform DirectionLight u_sunlight;

uniform sampler2D u_water_position;
uniform sampler2D u_water_normal;
uniform sampler2D u_water_color;

uniform float u_time;

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

float fbm(vec2 st) {
    int octaves = 4;
    float t = 0.;
    float a = 1., f = 1.;
    for (int i = 0; i < octaves; ++i) {
        t += a * noise(st * f);
        f *= 2.;
        a *= 0.5;
    }
    return t;
}

float get_shadow(float cos_theta) {
    vec4 frag_pos_light_space = u_light_space * texture(u_position, v_tex_coords);
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    // convert to [0, 1]
    proj_coords = proj_coords * 0.5 + 0.5;

    // closest depth to light
    float closest_depth = texture(u_depth_map, proj_coords.xy).x;
    // fragment depth
    float frag_depth = proj_coords.z;
    float bias = 0.0025 * tan(acos(cos_theta));
    vec2 texel_size = 1. / textureSize(u_depth_map, 0);

    float shadow = 0.0;
    int pcfno = 5;
    for (int y = -pcfno; y <= pcfno; ++y) {
        for (int x = -pcfno; x <= pcfno; ++x) {
            vec2 offset = vec2(x, y);
            float r = rand(offset * 2.32 + proj_coords.xy);
            offset = offset * texel_size * r * 1.89;
            // depth test
            float d = texture(u_depth_map, proj_coords.xy + offset).x;
            float s = (frag_depth - bias > d) ? 1. : 0.;
            shadow += s;
        }
    }
    int total = (pcfno * 2 + 1) * (pcfno * 2 + 1);

    return shadow / total;
}

vec3 get_sunlight(vec3 position, vec3 normal) {
    // vec3 position = texture(u_position, v_tex_coords).rgb;
    // vec3 normal = texture(u_normal, v_tex_coords).rgb;

    vec3 view_dir = normalize(u_view_pos - position);

    vec3 light_dir = normalize(u_sunlight.direction);
    float cos_theta = dot(normal, -light_dir);

    // ambient
    float ao = texture(u_ssao, v_tex_coords).r;
    ao = clamp(ao, 0.5, 1.0);
    vec3 ambient = u_sunlight.ambient * ao;
    // diffuse
    vec3 diffuse = max(cos_theta, 0.0) * u_sunlight.diffuse;
    // specular
    vec3 reflect_dir = reflect(light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = spec * u_sunlight.specular;

    vec3 result = ambient + (1.0 - get_shadow(cos_theta)) * (diffuse + specular);
    return result;
}

vec3 get_water(vec3 position, vec3 normal) {
    vec3 view_dir = normalize(u_view_pos - position);

    vec3 light_dir = normalize(u_sunlight.direction);
    float cos_theta = dot(normal, -light_dir);

    // ambient
    vec3 ambient = u_sunlight.ambient;
    // diffuse
    vec3 diffuse = max(cos_theta, 0.0) * u_sunlight.diffuse;
    // specular
    vec3 reflect_dir = reflect(light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 8.);
    vec3 specular = spec * u_sunlight.specular * 0.2;

    vec3 result = ambient + diffuse + specular;
    return result;
}

float map(float x, float a, float b, float c, float d) {
    return c + (x - a) * (d - c) / (b - a);
}

vec2 map2D(vec2 x, vec2 a, vec2 b, vec2 c, vec2 d) {
    return c + (x - a) * (d - c) / (b - a);
}

float dist_sq(vec3 a, vec3 b) {
    vec3 v = a - b;
    return dot(v, v);
}
const vec3 sky_color = vec3(135., 206., 235.) / 255.;

vec3 apply_fog(vec3 col, float t, vec3 rd, vec3 lig) {
    const float b = 0.004;
    float fog_amount = 1. - exp(-t*b);
    float sun_amount = max(dot(rd, lig), 0.);
    vec3 fog_col = mix(sky_color, vec3(1., 0.9, 0.7), pow(sun_amount, 8.));
    return mix(col, fog_col, fog_amount);
}

void main() {
    
    // block position
    vec3 pos = texture(u_position, v_tex_coords).xyz;
    vec3 normal = texture(u_normal, v_tex_coords).xyz;
    vec3 object_color = texture(u_color, v_tex_coords).rgb;

    // block lighting/final color
    vec3 block_lighting = get_sunlight(pos, normal) * object_color;

    // water position
    vec3 water_pos = texture(u_water_position, v_tex_coords).xyz;
    vec3 water_normal = texture(u_water_normal, v_tex_coords).xyz;
    vec3 water_color = texture(u_water_color, v_tex_coords).rgb;
    vec3 water_lighting = get_water(water_pos, water_normal) * water_color;

    // calculate the distance from the point to the camera
    float bd = dist_sq(pos, u_view_pos);
    float wd = dist_sq(water_pos, u_view_pos);

    // water is closer to camera & make sure there's actually water there
    if (wd < bd && water_color.rgb != vec3(0.)) {
        // mix the block and water lighting
        // the more specular the water, the less it should be transparent
        float blend = water_lighting.r + water_lighting.g + water_lighting.b;
        blend *= 0.33;
        blend *= 0.5;
        color.rgb = mix(block_lighting, water_lighting, 0.5 + blend);
        // edges between the block and the water
        float dist = abs(bd - wd);
        float r = 5;
        color.rgb += (1. - smoothstep(0., r, dist)) * 0.3;
    } else {
        color.rgb = block_lighting;
    }
    color.a = 1.;

    // calculate sky color
    // HACK: bad hack should be: sky if the z > 1.0
    // assumes no object will ever be fully black
    float s = 0.1;
    if (object_color.r < s && object_color.g < s && object_color.b < s) {
        color = vec4(sky_color, 1.);
    }

    // fog
    float fog_max_dist = 250.;
    float fog_min_dist = 100.;

    vec3 view_dir = normalize(pos - u_view_pos);
    float dist = length(pos - u_view_pos);
    vec4 fog_color = vec4(sky_color, 1.0);

    float fog_factor = (fog_max_dist - dist) / (fog_max_dist - fog_min_dist);
    fog_factor = clamp(fog_factor, 0.0, 1.0);

    color = mix(fog_color, color, fog_factor);

    // gamma correction
    float gamma = 1.2; // actual gamma = 2.2
    color.rgb = pow(color.rgb, vec3(1.0 / gamma));
}
