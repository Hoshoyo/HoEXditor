#define STB_TRUETYPE_IMPLEMENTATION
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"
#include "stb_truetype.h"
#include "util.h"
#include "math/matrix.h"

#define ATLAS_SIZE 4096
#define LAST_CHAR 0x1000

#pragma pack(push)
typedef struct {
	vec3 position;
	vec2 texcoord;
} vertex3d;
#pragma pack(pop)

typedef struct {
	vertex3d v[4];
	u8 indices[6];
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
} quad;

typedef struct {
	GLuint atlas_texture;
	GLuint atlas_texture_uniform_location;
	GLuint font_shader;
	quad q;
	mat4 projection;
	s8 atlas_bitmap[ATLAS_SIZE * ATLAS_SIZE];
	stbtt_packedchar packedchar[LAST_CHAR - ' '];
} Font_Rendering;

Font_Rendering font_rendering = { 0 };

void create_quad(GLuint shader, float width, float height);

void recompile_font_shader()
{
	s64 vs_size = 0;
	s64 fs_size = 0;
	u8* v_shader = read_entire_file("src/shaders/v_shader.vs", &vs_size);
	u8* f_shader = read_entire_file("src/shaders/f_shader.fs", &fs_size);
	if (!v_shader || !f_shader) {
		error_warning("Shader Error: shader files not found.\n");
		if (v_shader) hfree(v_shader);
		if (f_shader) hfree(f_shader);
	} else {
		glDeleteProgram(font_rendering.font_shader);
		font_rendering.font_shader = load_shader(v_shader, f_shader, vs_size, fs_size);
		hfree(v_shader);
		hfree(f_shader);
		log_success("Shader loaded successfully!\n");
	}
}

void my_stbtt_initfont(u8* filename, s32 font_size)
{
	u8* ttf_buffer = read_entire_file(filename, 0);
	stbtt_pack_context context;

	if (!stbtt_PackBegin(&context, font_rendering.atlas_bitmap, ATLAS_SIZE, ATLAS_SIZE, 0, 1, 0))
		error_fatal("Failed to initialize font", 0);

	stbtt_PackSetOversampling(&context, 2, 2);
	
	if (!stbtt_PackFontRange(&context, ttf_buffer, 0, font_size, ' ', LAST_CHAR - ' ', font_rendering.packedchar))
		error_fatal("Failed to pack font", 0);

	stbtt_PackEnd(&context);
	hfree(ttf_buffer);

	recompile_font_shader();

	font_rendering.projection = make_ortho(0.0f, 1024.0f, 0.0f, 768.0f);

	glGenTextures(1, &font_rendering.atlas_texture);
	glBindTexture(GL_TEXTURE_2D, font_rendering.atlas_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, ATLAS_SIZE, ATLAS_SIZE, 0, GL_ALPHA, GL_UNSIGNED_BYTE, font_rendering.atlas_bitmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	font_rendering.atlas_texture_uniform_location = glGetUniformLocation(font_rendering.font_shader, "texture_sample");

	create_quad(font_rendering.font_shader, 1.0f, 1.0f);
}

void create_quad(GLuint shader, float width, float height)
{
	vertex3d* v = font_rendering.q.v;
	v[0] = (vertex3d) { (vec3) {  0.0f,   0.0f, 0.0f }, (vec2) { 0.0f, 1.0f } };
	v[1] = (vertex3d) { (vec3) { width,   0.0f, 0.0f }, (vec2) { 1.0f, 1.0f } };
	v[2] = (vertex3d) { (vec3) {  0.0f, height, 0.0f }, (vec2) { 0.0f, 0.0f } };
	v[3] = (vertex3d) { (vec3) { width, height, 0.0f }, (vec2) { 1.0f, 0.0f } };

	s32 num_indices = 6;
	
	font_rendering.q.indices[0] = 0;
	font_rendering.q.indices[1] = 1;
	font_rendering.q.indices[2] = 2;
	font_rendering.q.indices[3] = 2;
	font_rendering.q.indices[4] = 1;
	font_rendering.q.indices[5] = 3;


	glGenVertexArrays(1, &font_rendering.q.vao);
	glBindVertexArray(font_rendering.q.vao);

	glGenBuffers(1, &font_rendering.q.ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, font_rendering.q.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(font_rendering.q.indices), font_rendering.q.indices, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &font_rendering.q.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, font_rendering.q.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(font_rendering.q.v), font_rendering.q.v, GL_STATIC_DRAW);

	GLint pos_loc = glGetAttribLocation(shader, "pos");
	GLint tco_loc = glGetAttribLocation(shader, "texcoord");

	glEnableVertexAttribArray(pos_loc);
	glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vertex3d), (void*)0);

	glEnableVertexAttribArray(tco_loc);
	glVertexAttribPointer(tco_loc, 2, GL_FLOAT, GL_FALSE, sizeof(vertex3d), (void*)&((vertex3d*)0)->texcoord);

}

void my_stbtt_print(float x, float y, char *text)
{
	// assume orthographic projection with units = screen pixels, origin at top left
	glUseProgram(font_rendering.font_shader);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDisable(GL_DEPTH_TEST);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glUniform1i(font_rendering.atlas_texture_uniform_location, 0);
	GLuint ploc = glGetUniformLocation(font_rendering.font_shader, "proj_matrix");
	glUniformMatrix4fv(ploc, 1, GL_TRUE, &font_rendering.projection.matrix[0][0]);

	const float ss = 800.0f;
	vertex3d v[4];
	v[0] = (vertex3d) { (vec3) { -ss, -ss, 0.0f }, (vec2) { 0.0f, 1.0f } };
	v[1] = (vertex3d) { (vec3) {  ss, -ss, 0.0f }, (vec2) { 1.0f, 1.0f } };
	v[2] = (vertex3d) { (vec3) { -ss,  ss, 0.0f }, (vec2) { 0.0f, 0.0f } };
	v[3] = (vertex3d) { (vec3) {  ss,  ss, 0.0f }, (vec2) { 1.0f, 0.0f } };

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(v), v);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
	
	glBindVertexArray(font_rendering.q.vao);
	glBindTexture(GL_TEXTURE_2D, font_rendering.atlas_texture);

	u8* text_ = "Hello World!";

	float pos_x = 1024.0f / 2.0f;
	float pos_y = 768.0f / 2.0f;
	float offsetX = 0, offsetY = 0;
	for(int i = 0; text_[i] != 0; ++i)
	{
		stbtt_aligned_quad quad;
		stbtt_GetPackedQuad(font_rendering.packedchar, ATLAS_SIZE, ATLAS_SIZE, 0xac - ' ', &offsetX, &offsetY, &quad, 1);
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
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
	}
	glDisable(GL_BLEND);
}
