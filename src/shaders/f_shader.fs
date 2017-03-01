#version 330 core
in vec2 uv;
out vec4 frag_color;

uniform sampler2D texture_sample;
uniform vec4 font_color;
uniform bool use_texture = true;
uniform bool use_solid_color = true;

void main(){
	vec4 color = font_color;
	vec4 tc = texture(texture_sample, uv);
	if(use_texture){
		if(use_solid_color){
			frag_color = vec4(color.rgb, tc.a);
		} else {
			frag_color = tc;
		}
	} else {
		frag_color = font_color;
	}
	//frag_color = vec4(color.xyz, texture(texture_sample, uv).r * color.a);
}