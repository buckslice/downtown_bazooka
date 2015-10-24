#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in vec3 color;
layout (location = 3) in mat4 instanceMatrix;

out vec2 Texcoord;
out vec3 Color;

uniform mat4 proj;
uniform mat4 view;

void main()
{
    Texcoord = texcoord;
	Color = color;
    gl_Position = proj * view * instanceMatrix * vec4(position, 1.0f); 
}