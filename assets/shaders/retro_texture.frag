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
const float sharpness = 0.75;

// How many are misalign the color beams
const float misalign = 0.4;

// how much a scanline should darken its line, 0-1
const float scanIntensity = 0.1;

// amount of noise, 0-1 (a little goes a long way)
const float noise = 0.075;

// Monitor curvature
const float distortion = 0.08;

// Flicker intesity
//const float flicker = 0.025;

// Time depedent FX
uniform float time = 0.0;

// Screen texture
uniform sampler2D texture0;

// how much to boost the brightness of pixels to compensate for scanlines, 0-1
// 0 = no increase, 1 = double brightness
uniform float brightness = 0.0;

// Calcs new UV coords to reproduce a curvature distorsion
vec2 barrelDistortion(vec2 coord) {
  vec2 cc = coord - 0.5;
  float dist = dot(cc, cc) * distortion;
  return coord + cc * (1.0 - dist) * dist;
}

void main(void) {
    // Apply curvature fx
    vec2 uv = barrelDistortion(ex_UV);
    if ( (uv.x <0.0 || uv.x > 1.0) || (uv.y <0.0 || uv.y > 1.0)) {
      // Ignore fragments that are outside of the screen
      discard;
      return;
    }

    // Precalculate misalign in function of horizontal pos
    float mis = misalign * pixelSize.x * (misalign + (1.0 - misalign)*2 * abs(uv.x - 0.5));

    // Precalculate bilinear filter things
    float xInc = pixelSize.x * (1.0 - sharpness) / 2.0;
    float yInc = pixelSize.y * (1.0 - sharpness) / 2.0;
    vec3 middle[4];
    vec2 uvs[4];
    uvs[0] = uv + vec2(-xInc, -yInc);
    uvs[1] = uv + vec2(xInc, -yInc);
    uvs[2] = uv + vec2(-xInc, yInc);
    uvs[3] = uv + vec2(xInc, yInc);

    for (int i=0; i < 4; i++) {
      middle[i] = texture(texture0, uvs[i] ).rgb;

      // Generate color border (misaligment)
      vec3 col;
      col.r = texture(texture0,vec2(uvs[i].x + mis ,uvs[i].y)).x;
      col.g = texture(texture0,vec2(uvs[i].x       ,uvs[i].y)).y;
      col.b = texture(texture0,vec2(uvs[i].x - mis ,uvs[i].y)).z;
      middle[i] = middle[i]*0.2 + col*0.8;

      // scanlines
      if (scanIntensity > 0.0 && mod(uvs[i].y, pixelSize.y ) > (pixelSize.y/2)) {
              middle[i].r = max(middle[i].r - scanIntensity, 0);
              middle[i].g = max(middle[i].g - scanIntensity, 0);
              middle[i].b = max(middle[i].b - scanIntensity, 0);
      };

    }

    // Apply bilinear filter over the composed image of orig texture + color misalign + scanline
    vec3 result = (middle[0] + middle[1] + middle[2] + middle[3]) * 0.25;

    // noise
    result = result + noise * fract(sin(dot(uv.xy , vec2(12.9898 + time, 78.233 + tan(time)))) * 43758.5453);

    // contrast curve
    //result.xyz = clamp(0.5*result.xyz + 0.5*result.xyz * 1.2*result.xyz, 0.0 , 1.0);

    //flickering (semi-randomized)
    // result *= 1.0 - flicker * fract(sin(dot(vec2(1.0) , vec2(12.9898 + time, 78.233 + tan(time)))) * 43758.5453);

    out_Color.xyz = result * (1.0 + brightness);
    out_Color.w = 1.0;

}
