// Basic Fragment Shader
#version 140

precision highp float; // needed only for version 1.30

in  vec3 ex_Color;

out vec4 out_Color;
 
void main(void) {
    out_Color.xyz = ex_Color.xyz;
    out_Color.w = 1.0;
}
