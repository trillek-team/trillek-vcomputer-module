// Basic Fragment Shader that generates a Scanline/TV effect
// Based on :
//   https://github.com/tacoe/detroit/blob/master/arthur/scanlines.glsl
//   http://pastebin.com/sTk3EUAp
//   https://gitorious.org/bsnes/bsnes/source/4246a6b0eb664d00ed3cac186d113cca4701911e:shaders/GLSL/Curvature.shader#L20
//   http://pastebin.com/TcPdyik0
#version 140

precision highp float; // needed only for version 1.30

// Base Color/Tint of the screen thing
//in vec3 ex_Color;
// UV coord of the screen thing
in vec2 ex_UV;

out vec4 out_Color;

// width of a pixel in texture units,
// should be set to 1 / width, 1 / height.
uniform vec2 pixelSize =  vec2(1.0/(320.0), 1.0/(240.0) );
 
// how sharp the bilinear filter is, 0 - 1
uniform float sharpness = 0.5;

// How many are misalign the color beams
uniform float misalign = 0.4;

// how much to boost the brightness of pixels to compensate for scanlines, 0-1
// 0 = no increase, 1 = double brightness
uniform float brightness = 0.0;
 
// how much a scanline should darken its line, 0-1
uniform float scanIntensity = 0.125;

// amount of noise, 0-1 (a little goes a long way)
uniform float noise = 0.075;

// Flicker intesity
//uniform float flicker = 0.07;
 
// Time depedent FX
uniform float time = 0.0;

// Screen texture
uniform sampler2D texture0;

#define PI            3.14159265
#define distortion    0.2
vec2 barrelDistortion(vec2 coord) {
  vec2 cc = coord - 0.5;
  float dist = dot(cc, cc) * distortion;
  return coord + cc * (1.0 - dist) * dist; 
}

float rand(vec2 co) {
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main(void) {
    // Apply curvature fx
    vec2 uv = barrelDistortion(ex_UV);
    if ( (uv.x <0 || uv.x > 1.0) || (uv.y <0 || uv.y > 1.0)) {
      discard;
      return;
    }

    // bilinear filter
    float xInc = pixelSize.x * (1.0 - sharpness) / 4.0;
    float yInc = pixelSize.y * (1.0 - sharpness) / 4.0;

    vec3 result = texture(texture0, uv + vec2(-xInc, -yInc)).rgb;
    result     += texture(texture0, uv + vec2(xInc, -yInc)).rgb;
    result     += texture(texture0, uv + vec2(-xInc, yInc)).rgb;
    result     += texture(texture0, uv + vec2(xInc, yInc)).rgb;
    result     /= 4.0;
    
    // Generate color border (misaligment)
    vec3 col;
    float mis = misalign * pixelSize.x * (misalign + (1.0 - misalign)*2 * abs(uv.x - 0.5));

    col.r = texture(texture0,vec2(uv.x + mis ,uv.y)).x;
    col.g = texture(texture0,vec2(uv.x       ,uv.y)).y;
    col.b = texture(texture0,vec2(uv.x - mis ,uv.y)).z;

    result = mix (result, col, 0.5);

    // noise
    result = result + noise * fract(sin(dot(uv.xy , vec2(12.9898 + time, 78.233 + tan(time)))) * 43758.5453);

    // contrast curve
    result.xyz = clamp(result.xyz*0.5 + 0.5*result.xyz*result.xyz *1.2, 0.0 , 1.0);
    
    // scanlines
    if (scanIntensity > 0.0 && mod(uv.y, pixelSize.y ) > (pixelSize.y/2)) {
            result.r = max(result.r - scanIntensity, 0);
            result.g = max(result.g - scanIntensity, 0);
            result.b = max(result.b - scanIntensity, 0);
    };

    //flickering (semi-randomized)
    //result *= clamp(1.0 - flicker * rand(vec2(time, 1)), 0.7, 1.0);

    out_Color.xyz = result * (1.0 + brightness);
    out_Color.w = 1.0;
    
}
