#shader vertex
#version 450

layout(location=0) in int a_data;
       
uniform mat4 u_light_space;

uniform vec3 u_chunk_offset;
uniform vec3 u_chunk_size;

void main() {
    // compute world offset
    vec3 pos = vec3(
        float((a_data & 0xF) >> 0),
        float((a_data & 0x7F0) >> 4),
        float((a_data & 0x7800) >> 11)
    );
    pos += u_chunk_offset * u_chunk_size;

    gl_Position = u_light_space * vec4(pos, 1.0);
}

#shader fragment
#version 450

void main() {

}
