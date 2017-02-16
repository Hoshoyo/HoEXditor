#define STB_TRUETYPE_IMPLEMENTATION
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"
#include "stb_truetype.h"
#include "util.h"

#define ATLAS_SIZE 1024
unsigned char temp_bitmap[ATLAS_SIZE * ATLAS_SIZE];
stbtt_packedchar packedchar[512 - ' '];
GLuint texture;
GLuint shader;

#pragma pack(push)
typedef struct {
	vec3 position;
	vec2 texcoord;
} vertex3d;
#pragma pack(pop)

void create_quad(GLuint shader, float width, float height);

void recompile_shader()
{
	s64 vs_size = 0;
	s64 fs_size = 0;
	u8* v_shader = read_entire_file("src/shaders/v_shader.vs", &vs_size);
	u8* f_shader = read_entire_file("src/shaders/f_shader.fs", &fs_size);
	if (!v_shader || !f_shader) {
		error_warning("Shader Error: shader files not found.\n");
	} else {
		glDeleteProgram(shader);
		shader = load_shader(v_shader, f_shader, vs_size, fs_size);
		hfree(v_shader);
		hfree(f_shader);
		log_success("Shader loaded successfully!\n");
	}
}

GLuint texture_loc;
void my_stbtt_initfont(u8* filename, s32 font_size)
{
	u8* ttf_buffer = read_entire_file(filename, 0);
	stbtt_pack_context context;

	if (!stbtt_PackBegin(&context, temp_bitmap, ATLAS_SIZE, ATLAS_SIZE, 0, 1, 0))
		error_fatal("Failed to initialize font", 0);

	stbtt_PackSetOversampling(&context, 2, 2);
	if (!stbtt_PackFontRange(&context, ttf_buffer, 0, font_size, ' ', 512 - ' ', packedchar))
		error_fatal("Failed to pack font", 0);

	stbtt_PackEnd(&context);
	recompile_shader();

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, ATLAS_SIZE, ATLAS_SIZE, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	texture_loc = glGetUniformLocation(shader, "texture_sample");

	create_quad(shader, 1.0f, 1.0f);
}

GLuint vao;
GLuint vbo;
GLuint ebo;


void create_quad(GLuint shader, float width, float height)
{
	vertex3d v[4];
	v[0] = (vertex3d) { (vec3) {  0.0f,   0.0f, 0.0f }, (vec2) { 0.0f, 1.0f } };
	v[1] = (vertex3d) { (vec3) { width,   0.0f, 0.0f }, (vec2) { 1.0f, 1.0f } };
	v[2] = (vertex3d) { (vec3) {  0.0f, height, 0.0f }, (vec2) { 0.0f, 0.0f } };
	v[3] = (vertex3d) { (vec3) { width, height, 0.0f }, (vec2) { 1.0f, 0.0f } };

	s32 num_indices = 6;

	u32 indices[] = { 0, 1, 2, 2, 1, 3 };

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);

	GLint pos_loc = glGetAttribLocation(shader, "pos");
	GLint tco_loc = glGetAttribLocation(shader, "texcoord");

	glEnableVertexAttribArray(pos_loc);
	glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vertex3d), (void*)0);

	glEnableVertexAttribArray(tco_loc);
	glVertexAttribPointer(tco_loc, 2, GL_FLOAT, GL_FALSE, sizeof(vertex3d), (void*)&((vertex3d*)0)->texcoord);

}

typedef struct{
	float matrix[4][4];
}mat4;

mat4 make_ortho(float left, float right, float bottom, float top, float n, float f)
{
	mat4 result;
	/*
	result.matrix[0][0] = 2.0f / (right - left);	result.matrix[0][1] = 0;						result.matrix[0][2] = 0;					result.matrix[0][3] = -((right + left) / (right - left));
	result.matrix[1][0] = 0;						result.matrix[1][1] = 2.0f / (top - bottom);	result.matrix[1][2] = 0;					result.matrix[1][3] = -((top + bottom) / (top - bottom));
	result.matrix[2][0] = 0;						result.matrix[2][1] = 0;						result.matrix[2][2] = -2.0f / (f - n);		result.matrix[2][3] = -((f + n) / (f - n));
	result.matrix[3][0] = 0;						result.matrix[3][1] = 0;						result.matrix[3][2] = 0;					result.matrix[3][3] = 1;
	*/
	result.matrix[0][0] = 2.0f / (right - left);	result.matrix[0][1] = 0;						result.matrix[0][2] = 0;		result.matrix[0][3] = -((right + left) / (right - left));
	result.matrix[1][0] = 0;						result.matrix[1][1] = 2.0f / (top - bottom);	result.matrix[1][2] = 0;		result.matrix[1][3] = -((top + bottom) / (top - bottom));
	result.matrix[2][0] = 0;						result.matrix[2][1] = 0;						result.matrix[2][2] = -1.0f;	result.matrix[2][3] = 0;
	result.matrix[3][0] = 0;						result.matrix[3][1] = 0;						result.matrix[3][2] = 0;		result.matrix[3][3] = 1;

	/*
	
	Result[0][0] = valType(2) / (right - left);
	Result[1][1] = valType(2) / (top - bottom);
	Result[2][2] = - valType(1);
	Result[3][0] = - (right + left) / (right - left);
	Result[3][1] = - (top + bottom) / (top - bottom);
		
	*/

	return result;
}

mat4 make_ident()
{
	mat4 res = { 0 };
	res.matrix[0][0] = 1.0f;
	res.matrix[1][1] = 1.0f;
	res.matrix[2][2] = 1.0f;
	res.matrix[3][3] = 1.0f;
	return res;
}

void my_stbtt_print(float x, float y, char *text)
{
	mat4 ortho = make_ortho(0.0f, 1024.0f, 0.0f, 768.0f, 0.1f, 100.0f);
	
	// assume orthographic projection with units = screen pixels, origin at top left
	glUseProgram(shader);
	
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDisable(GL_DEPTH_TEST);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glUniform1i(texture_loc, 0);
	GLuint ploc = glGetUniformLocation(shader, "proj_matrix");
	glUniformMatrix4fv(ploc, 1, GL_TRUE, &ortho.matrix[0][0]);

	/*
	vertex3d v[4];
	v[0] = (vertex3d) { (vec3) { -400.0f, -400.0f + 200.0f, 0.0f }, (vec2) { 0.0f, 1.0f } };
	v[1] = (vertex3d) { (vec3) {  400.0f, -400.0f + 200.0f, 0.0f }, (vec2) { 1.0f, 1.0f } };
	v[2] = (vertex3d) { (vec3) { -400.0f,  400.0f + 200.0f, 0.0f }, (vec2) { 0.0f, 0.0f } };
	v[3] = (vertex3d) { (vec3) {  400.0f,  400.0f + 200.0f, 0.0f }, (vec2) { 1.0f, 0.0f } };

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(v), v);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	*/

	glBindVertexArray(vao);
	glBindTexture(GL_TEXTURE_2D, texture);

	unsigned char text_[] = "Héllo World!";
	
	float pos_x = 1024.0f / 2.0f;
	float pos_y = 768.0f / 2.0f;
	float offsetX = 0, offsetY = 0;
	for(int i = 0; text_[i] != 0; ++i)
	{
		stbtt_aligned_quad quad;
		stbtt_GetPackedQuad(packedchar, ATLAS_SIZE, ATLAS_SIZE, text_[i] - ' ', &offsetX, &offsetY, &quad, 1);
		float xmin = quad.x0 + pos_x;
		float xmax = quad.x1 + pos_x;
		float ymin = -quad.y1 + pos_y;
		float ymax = -quad.y0 + pos_y;

		vertex3d v[4];
		v[0] = (vertex3d) { (vec3) { xmin, ymin, 0.0f }, (vec2) { quad.s0, quad.t1 } };
		v[1] = (vertex3d) { (vec3) { xmax, ymin, 0.0f }, (vec2) { quad.s1, quad.t1 } };
		v[2] = (vertex3d) { (vec3) { xmin, ymax, 0.0f }, (vec2) { quad.s0, quad.t0 } };
		v[3] = (vertex3d) { (vec3) { xmax, ymax, 0.0f }, (vec2) { quad.s1, quad.t0 } };

		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(v), v);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
	glDisable(GL_BLEND);
}
