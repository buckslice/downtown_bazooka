#version 330 core

in vec2 Texcoord;
in vec3 Color;

out vec4 color;

uniform sampler2D tex;
uniform sampler2D noise;
uniform float time;
uniform float tileFactor;

in vec3 wpos;

void main()
{
    vec2 ntc = Texcoord*5.0;
    ntc.x += time*0.1;
    ntc.y += time*0.1;
    vec4 uvx = texture(noise, ntc);
    ntc.x += 0.5;
    //ntc.y += 0.5;
    vec4 uvy = texture(noise, ntc);

    vec2 uvoff = (vec2(uvx.r, uvy.r) - vec2(0.5)) * .1;

    color = texture(tex, Texcoord*tileFactor + uvoff) * vec4(Color,1.0);

}