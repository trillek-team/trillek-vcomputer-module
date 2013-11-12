// Basic Vertex Shader
#version 140 
 
in  vec3 in_Position;
in  vec3 in_Color;
out vec3 ex_Color;
 
void main(void) {
    gl_Position.xyz = in_Position.xyz;
    gl_Position.w = 1.0;

    ex_Color = vec3(1.0, 0.0, 0.0);
}

