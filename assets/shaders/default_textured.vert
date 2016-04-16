#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texcoord;

out vec3 Color;
out vec2 Texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 wpos;

void main()
{
	Color = color;
	Texcoord = texcoord;
	vec4 mvec = model*vec4(position, 1.0);
	wpos = mvec.xyz;
    gl_Position = proj * view * mvec;
}