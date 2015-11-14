#version 330 core
in vec2 Texcoord;
out vec4 color;

uniform sampler2D screen;

void main()
{ 
    color = texture(screen, Texcoord);
}