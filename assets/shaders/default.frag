#version 330 core

in vec2 Texcoord;
in vec3 Color;

out vec4 color;

uniform sampler2D tex;

void main()
{
    color = texture(tex, Texcoord) * vec4(Color, 1.0);
}