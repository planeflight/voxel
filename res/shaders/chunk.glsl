#shader vertex
#version 450

layout(location=0) in vec3 a_Pos;

void main() {
    gl_Position = u_ModelView * vec3(a_Pos);
}

#shader fragment
#version 450



void main() {

}

