#version 330 core
in vec2 uv;
out vec4 frag_color;

uniform sampler2D texture_sample;
uniform vec4 font_color;

void main(){
	vec4 color = font_color;//vec4(0.2, 0.9, 0.55, 1.0);
	vec4 tc = texture(texture_sample, uv);
	frag_color = vec4(color.xyz, tc.a);
	//frag_color = vec4(color.xyz, texture(texture_sample, uv).r * color.a);
}