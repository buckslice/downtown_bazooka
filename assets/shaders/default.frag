#version 330 core

in vec2 Texcoord;
in vec3 Color;

out vec4 color;

uniform sampler2D tex;

in vec3 wpos;

void main()
{
	color = texture(tex, Texcoord) * vec4(Color,1.0);

	// ignores texture
	//color = vec4(Color,1.0);
	
	// enable one of these qs and the last color line for different effects
	//float q = step(mod(gl_FragCoord.z / gl_FragCoord.w, 3.0), 0.3);
	//float q = step(mod(wpos.x + wpos.z, 10.0), 2.0);
	//float q = step(mod(wpos.y, 5.0), 0.5);
	
	//color = vec4(vec3(q)*Color, 1.0f);
}