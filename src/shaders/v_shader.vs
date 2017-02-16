#version 330 core
in vec3 pos;
in vec2 texcoord;
out vec2 uv;

uniform mat4 proj_matrix;

void main(){
	gl_Position = proj_matrix * vec4(pos, 1.0);
	uv = texcoord;
}