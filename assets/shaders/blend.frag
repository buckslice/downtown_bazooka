#version 330 core
out vec4 FragColor;
in vec2 Texcoord;

uniform sampler2D scene;
uniform sampler2D blur;
uniform float exposure;

void main()
{             
    const float gamma = 2.2;
    vec3 sceneColor = texture(scene, Texcoord).rgb;      
    vec3 blurColor = texture(blur, Texcoord).rgb;
    sceneColor += blurColor; // additive blending
	
    // tone mapping
    //vec3 result = vec3(1.0) - exp(-sceneColor * exposure);
	
    // also gamma correct while we're at it       
    //result = pow(result, vec3(1.0 / gamma));
    //FragColor = vec4(result, 1.0f);
	
	FragColor = vec4(sceneColor, 1.0f);
}  