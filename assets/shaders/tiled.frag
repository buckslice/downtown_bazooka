#version 330 core

in vec2 Texcoord;
//uniform vec3 Color;

out vec4 color;

uniform sampler2D tex;

void main()
{
	color = texture(tex, Texcoord*100.0f) * vec4(0.1,0.0,0.2,1.0);
	
}
