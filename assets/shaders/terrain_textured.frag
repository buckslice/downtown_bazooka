#version 330 core

in vec2 Texcoord;
in vec3 Color;

out vec4 color;

uniform sampler2D tex;
uniform sampler2D noise;
uniform float time;
uniform float clerp;
uniform float tileFactor;

in vec3 wpos;

void main()
{
    //normal ground

    // vec2 ntc = Texcoord*5.0;
    // ntc.x += time*0.05;
    // ntc.y += time*0.05;
    // vec4 uvx = texture(noise, ntc);
    // vec2 uvoff = (vec2(uvx.r, uvx.r) - vec2(0.5)) * .1;

    // color = texture(tex, Texcoord*tileFactor + uvoff) * vec4(Color,1.0);


    // lava ground

    //cheaper lava
    vec2 ntc = Texcoord*5.0;
    ntc.x += time*0.1;
    ntc.y += time*0.1;
    vec4 uvx = texture(noise, ntc);
    vec2 uvoff = (vec2(uvx.r, uvx.r) - vec2(0.5))*0.1;
    ntc = Texcoord;
    ntc.x -= time*0.01;
    ntc.y -= time*0.01;
    uvx = texture(noise, ntc);
    vec2 uvoff2 = (vec2(uvx.r, uvx.r) - vec2(0.5))*0.1;

    // expensive lava
    // vec2 ntc = Texcoord*5.0;
    // ntc.x += time*0.1;
    // ntc.y += time*0.1;
    // vec4 uvx = texture(noise, ntc);
    // ntc.x += 0.5;
    // ntc.y += 0.5;
    // vec4 uvy = texture(noise, ntc);
    // vec2 uvoff = (vec2(uvx.r, uvy.r) - vec2(0.5))*0.1;
    // ntc = Texcoord * 1.0;
    // ntc.x -= time*0.01;
    // ntc.y -= time*0.01;
    // uvx = texture(noise, ntc);
    // ntc.y += 0.5;
    // uvy = texture(noise, ntc);
    // vec2 uvoff2 = (vec2(uvx.r, uvy.r) - vec2(0.5))*0.1;

    vec4 c1 = texture(tex, Texcoord*tileFactor + uvoff)*0.5;
    vec4 c2 = texture(tex, ((Texcoord+uvoff2)*tileFactor));
    vec4 tcolor = mix(vec4(1.0, 0.0, 0.0, 1.0), vec4(Color, 1.0), clerp);

    color = clamp(c1+c2, 0.0, 1.0) * tcolor;

}