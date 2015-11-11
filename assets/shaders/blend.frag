#version 330 core
out vec4 FragColor;
in vec2 Texcoord;

uniform sampler2D scene;
uniform sampler2D blur;
uniform float blurStrength;

void main()
{             
    const float gamma = 2.2;
    vec3 sceneColor = texture(scene, Texcoord).rgb;      
    vec3 blurColor = texture(blur, Texcoord).rgb;
    sceneColor += blurColor * blurStrength; // additive blending
	
    FragColor = vec4(sceneColor, 1.0f);
	
}  