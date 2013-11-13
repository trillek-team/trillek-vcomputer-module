// Basic Fragment Shader that uses textures
#version 140

precision highp float; // needed only for version 1.30

in  vec3 ex_Color;
in vec2 ex_UV;

out vec4 out_Color;

uniform sampler2D texture0;

void main(void) {
    out_Color.xyz = ex_Color * texture( texture0, ex_UV ).rgb;
    out_Color.w = 1.0;
}
