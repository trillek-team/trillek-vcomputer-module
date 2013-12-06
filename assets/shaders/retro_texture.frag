// Basic Fragment Shader that uses textures
#version 140

precision highp float; // needed only for version 1.30

in vec3 ex_Color;
in vec2 ex_UV;

out vec4 out_Color;

uniform sampler2D texture0;

float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main(void) {
    float time = 0.0;
    //vec2 q = gl_FragCoord.xy /vec2(800.0, 600.0);
    vec2 q = ex_UV.xy; // /vec2(320.0, 240.0);

    // subtle zoom in/out
    vec2 uv = q; //0.5 + (q-0.5)*(0.98 + 0.006*sin(0.9*time));

    vec3 oricol = texture(texture0,vec2(q.x,q.y)).xyz;
    vec3 col;

    // start with the source texture and misalign the rays it a bit
    // TODO animate misalignment upon hit or similar event
    col.r = texture(texture0,vec2(uv.x+0.003,uv.y)).x;
    col.g = texture(texture0,vec2(uv.x,uv.y)).y;
    col.b = texture(texture0,vec2(uv.x-0.003,uv.y)).z;

    // contrast curve
    col = clamp(col*0.5+0.5*col*col*1.2,0.0,1.0);

    //vignette
    col *= 0.6 + 0.4*16.0*uv.x*uv.y*(1.0-uv.x)*uv.y;

    //color tint
    col *= vec3(0.9,1.0,0.9);

    //scanline (last 2 constants are crawl speed and size)
    //TODO make size dependent on viewport
    col *= 0.8+0.2*sin(uv.y*900.0);

    //flickering (semi-randomized)
    //col *= 1.0-0.07*rand(vec2(time, tan(time)));

    //smoothen
    //float comp = smoothstep( 0.2, 0.7, sin(time) );
    //col = mix( col, oricol, clamp(-2.0+2.0*q.x+3.0*comp,0.0,1.0) );
    col = mix( col, oricol, 0.5 );

    out_Color = vec4(col,1.0);
    
    //out_Color.xyz = ex_Color * texture( texture0, ex_UV ).rgb;
    //out_Color.w = 1.0;
}
