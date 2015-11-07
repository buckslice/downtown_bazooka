#version 330 core

in vec2 Texcoord;
in vec3 Color;

out vec4 color;

uniform sampler2D tex;

void main()
{
	color = texture(tex, Texcoord) * vec4(Color,1.0);
	/*for(int i = 1; i <= 10; i++){
		float a = i*.01;
		color = color+(texture(tex,Texcoord+vec2(0,-a))+texture(tex,Texcoord+vec2(a,0))+texture(tex,Texcoord+vec2(0,a))+texture(tex,Texcoord+vec2(-a,0)))/i*.025f;
	}*/
    /*color = (texture(tex, Texcoord)
	+ (texture(tex,Texcoord+vec2(0,-.05))+texture(tex,Texcoord+vec2(.05,0))+texture(tex,Texcoord+vec2(0,.05))+texture(tex,Texcoord+vec2(-.05,0)))*.25
	+ (texture(tex,Texcoord+vec2(0,-.1))+texture(tex,Texcoord+vec2(.1,0))+texture(tex,Texcoord+vec2(0,.1))+texture(tex,Texcoord+vec2(-.1,0)))*.05
	) * vec4(Color, 1.0);*/
}
