#version 330 core
in vec2 uv;
out vec4 frag_color;

uniform sampler2D texture_sample;

void main(){
	vec4 color = vec4(1.0, 1.0, 0.0, 1.0);
	vec4 tc = texture(texture_sample, uv);
	frag_color = vec4(color.xyz, tc.a);
	//frag_color = vec4(color.xyz, texture(texture_sample, uv).r * color.a);
}