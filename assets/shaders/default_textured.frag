#version 330 core

in vec2 Texcoord;
in vec3 Color;

out vec4 color;

uniform sampler2D tex;

in vec3 wpos;

void main()
{
    // vec4 c1 = texture(tex, Texcoord);
    // vec4 c2 = texture(tex, Texcoord* 10.0f);
    // //c2 = vec4(0.0);
    // if(c1.r < 0.01 && c2.r < 0.01){
    //     color = vec4(0.0);
    // }else{
    // color = clamp(c1+c2, 0.0, 1.0) * vec4(Color,1.0);}

    color = texture(tex, Texcoord) * vec4(Color,1.0);

    // ignores texture
    //color = vec4(Color,1.0);
    
    // enable one of these qs and the last color line for different effects
    //float q = step(mod(gl_FragCoord.z / gl_FragCoord.w, 3.0), 0.3);
    //float q = step(mod(wpos.x + wpos.z, 10.0), 2.0);
    //float q = step(mod(wpos.y, 5.0), 0.5);    // vertical strips
    //color = vec4(vec3(q)*Color, 1.0f);
}