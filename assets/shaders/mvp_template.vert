// Basic Vertex Shader Template with Model, View, Project matrixes 
#version 140 
 
in  vec3 in_Position;
in  vec3 in_Color;
in  vec3 in_Normal;

in  vec2 in_UV;

uniform mat4 in_Model;
uniform mat4 in_View;
uniform mat4 in_Proj;

out vec3 ex_Color;
out vec3 ex_Normal;
out vec2 ex_UV;
 
void main(void) {

    ex_Color = vec3(1.0, 0.5, 0.0);
    gl_Position = in_Proj * (in_View * (in_Model * vec4(in_Position, 1.0)));
    //ex_Color = vec3(1.0, 1.0, 1.0) * vec3(in_Color);
    ex_Normal = (in_Model * vec4(in_Normal, 0)).xyz;
    ex_UV = in_UV;
}

