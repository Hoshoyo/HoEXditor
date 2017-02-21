#include "font_rendering.h"
#include "stb_rect_pack.h"
#include "stb_truetype.h"

Font_Rendering font_rendering = { 0 };
Debug_Font_Rendering debug_font_rendering = { 0 };

void make_entity(GLuint shader, GLuint* vao, GLuint* vbo, GLuint* ebo, void* indices, int size_indices, u32 hint)
{
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);

	glGenBuffers(1, ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_indices, indices, GL_DYNAMIC_DRAW);

	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(font_rendering.q.v), font_rendering.q.v, GL_DYNAMIC_DRAW);

	font_rendering.attrib_pos_loc = glGetAttribLocation(shader, "pos");
	font_rendering.attrib_texcoord_loc = glGetAttribLocation(shader, "texcoord");

	glEnableVertexAttribArray(font_rendering.attrib_pos_loc);
	glVertexAttribPointer(font_rendering.attrib_pos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vertex3d), (void*)0);

	glEnableVertexAttribArray(font_rendering.attrib_texcoord_loc);
	glVertexAttribPointer(font_rendering.attrib_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(vertex3d), (void*)&((vertex3d*)0)->texcoord);
}

void create_quad(GLuint shader, float width, float height)
{
	vertex3d* v = font_rendering.q.v;
	v[0] = (vertex3d) { (vec3) { 0.0f, 0.0f, 0.0f }, (vec2) { 0.0f, 1.0f } };
	v[1] = (vertex3d) { (vec3) { width, 0.0f, 0.0f }, (vec2) { 1.0f, 1.0f } };
	v[2] = (vertex3d) { (vec3) { 0.0f, height, 0.0f }, (vec2) { 0.0f, 0.0f } };
	v[3] = (vertex3d) { (vec3) { width, height, 0.0f }, (vec2) { 1.0f, 0.0f } };

	font_rendering.q.indices[0] = 0;
	font_rendering.q.indices[1] = 1;
	font_rendering.q.indices[2] = 2;
	font_rendering.q.indices[3] = 2;
	font_rendering.q.indices[4] = 1;
	font_rendering.q.indices[5] = 3;
	
	make_entity(shader, &font_rendering.q.vao, &font_rendering.q.vbo, &font_rendering.q.ebo, font_rendering.q.indices, 6 * sizeof(u8), GL_DYNAMIC_DRAW);
}

int recompile_font_shader()
{
	s64 vs_size = 0;
	s64 fs_size = 0;
	u8* v_shader = read_entire_file("src/shaders/v_shader.vs", &vs_size);
	u8* f_shader = read_entire_file("src/shaders/f_shader.fs", &fs_size);
	if (!v_shader || !f_shader) {
		error_warning("Shader Error: shader files not found.\n");
		if (v_shader) hfree(v_shader);
		if (f_shader) hfree(f_shader);
		return 0;
	}
	else {
		glDeleteProgram(font_rendering.font_shader);
		GLuint new_shader = load_shader(v_shader, f_shader, vs_size, fs_size);
		if (new_shader == -1) {
			error_warning("\nCould not compile font shader...\n");
			return 0;
		}
		font_rendering.font_shader = new_shader;
		hfree(v_shader);
		hfree(f_shader);

		font_rendering.atlas_texture_uniform_location = glGetUniformLocation(font_rendering.font_shader, "texture_sample");
		font_rendering.font_color_uniform_location = glGetUniformLocation(font_rendering.font_shader, "font_color");
		font_rendering.projection_uniform_location = glGetUniformLocation(font_rendering.font_shader, "proj_matrix");

		log_success("Shader loaded successfully!\n");
		return 1;
	}
}

void load_font(u8* filename, s32 font_size)
{
	u8* ttf_buffer = read_entire_file(filename, 0);
	font_rendering.ttf_buffer = ttf_buffer;

	stbtt_pack_context context;

	int err = stbtt_InitFont(&font_rendering.font_info, ttf_buffer, 0);
	if (!err) error_fatal("Fatal error, font could not be found.\n", 0);

	// get metrics
	int ascent = 0, descent = 0, lineGap = 0;
	stbtt_GetFontVMetrics(&font_rendering.font_info, &ascent, &descent, &lineGap);
	float max_height = (ascent - descent + lineGap);
	float downsize = font_size * (1.0f / max_height);
	font_rendering.max_height = max_height * downsize;
	font_rendering.ascent = ascent * downsize;
	font_rendering.descent = descent * downsize;
	font_rendering.font_size = font_size;
	font_rendering.downsize = font_size * (1.0f / max_height);
	int x0, x1, y0, y1;
	stbtt_GetFontBoundingBox(&font_rendering.font_info, &x0, &y0, &x1, &y1);
	font_rendering.max_width = (x1 - x0) * font_rendering.downsize;

	// start packing on atlas
	if (!stbtt_PackBegin(&context, font_rendering.atlas_bitmap, ATLAS_SIZE, ATLAS_SIZE, 0, 1, 0))
		error_fatal("Failed to initialize font", 0);
	stbtt_PackSetOversampling(&context, 2, 2);
	if (!stbtt_PackFontRange(&context, ttf_buffer, 0, font_size, 0, LAST_CHAR, font_rendering.packedchar))
		error_fatal("Failed to pack font", 0);
	stbtt_PackEnd(&context);
	//hfree(ttf_buffer);

	glGenTextures(1, &font_rendering.atlas_texture);
	glBindTexture(GL_TEXTURE_2D, font_rendering.atlas_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, ATLAS_SIZE, ATLAS_SIZE, 0, GL_ALPHA, GL_UNSIGNED_BYTE, font_rendering.atlas_bitmap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	for (int i = 0; i < LAST_CHAR; ++i) {

		int x0, x1, y0, y1;
		int ret = stbtt_GetCodepointBox(&font_rendering.font_info, i, &x0, &y0, &x1, &y1);	// @todo make this cached
		if (ret == 0) {
			font_rendering.codepoint_width[i] = -1;
		} else {
			font_rendering.codepoint_width[i] = (x1 - x0) * font_rendering.downsize;
		}
	}
}

void init_font(u8* filename, s32 font_size, float win_width, float win_height)
{
	load_font(filename, font_size);

	if (recompile_font_shader() == -1) {
		error_fatal("Shader compilation error:\n", 0);
	}

	font_rendering.projection = make_ortho(0.0f, win_width, 0.0f, win_height);
	create_quad(font_rendering.font_shader, 1.0f, 1.0f);
}
void release_quad_buffers(quad *q)
{
	glDeleteBuffers(1, &q->vbo);
	glDeleteBuffers(1, &q->ebo);
	glDeleteVertexArrays(1, &q->vao);
}

void release_font()
{
	hfree(font_rendering.ttf_buffer);
	glDeleteTextures(1, &font_rendering.atlas_texture);
}


void update_font(float width, float height)
{
	font_rendering.projection = make_ortho(0.0f, width, 0.0f, height);
}

typedef struct {
	HWND window_handle;
	LONG win_width, win_height;
	WINDOWPLACEMENT g_wpPrev;
	HDC device_context;
	HGLRC rendering_context;
} Window_State;

extern Window_State win_state;

void render_transparent_quad(float minx, float miny, float maxx, float maxy, vec4* color)
{
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	vertex3d v[4];
	v[0] = (vertex3d) { (vec3) { minx, miny, 0.0f }, (vec2) { 0.0f, 1.0f } };
	v[1] = (vertex3d) { (vec3) { maxx, miny, 0.0f }, (vec2) { 1.0f, 1.0f } };
	v[2] = (vertex3d) { (vec3) { minx, maxy, 0.0f }, (vec2) { 0.0f, 0.0f } };
	v[3] = (vertex3d) { (vec3) { maxx, maxy, 0.0f }, (vec2) { 1.0f, 0.0f } };

	glUniform4fv(font_rendering.font_color_uniform_location, 1, (GLfloat*)color);
	glUniform1i(glGetUniformLocation(font_rendering.font_shader, "use_texture"), 0);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(v), v);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);

	glDisable(GL_BLEND);
}

int render_text(float x, float y, u8* text, s32 length, float max_width, vec4* color, Font_Render_Info* render_info)
{
	glUseProgram(font_rendering.font_shader);
	//glActiveTexture(GL_TEXTURE0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnableVertexAttribArray(font_rendering.attrib_pos_loc);
	glEnableVertexAttribArray(font_rendering.attrib_texcoord_loc);

	glUniform1i(font_rendering.atlas_texture_uniform_location, 0);
	glUniformMatrix4fv(font_rendering.projection_uniform_location, 1, GL_TRUE, &font_rendering.projection.matrix[0][0]);

	glBindVertexArray(font_rendering.q.vao);
	glBindTexture(GL_TEXTURE_2D, font_rendering.atlas_texture);

	int num_rendered = 0;
	float offx = 0, offy = 0;
	float prev_offx = 0;
	for (int i = 0; i < length; ++i, num_rendered++)
	{
		prev_offx = offx;
		stbtt_aligned_quad quad;
		stbtt_GetPackedQuad(font_rendering.packedchar, ATLAS_SIZE, ATLAS_SIZE, text[i], &offx, &offy, &quad, 1);

		float xmin = quad.x0 + x;
		float xmax = quad.x1 + x;
		float ymin = -quad.y1 + y;
		float ymax = -quad.y0 + y;

		// stop early if max width is reached
		if (render_info) {
			render_info->last_x = xmax;
			if (render_info->cursor_position == render_info->in_offset + i) {
				render_info->advance_x_cursor = x + prev_offx;
				render_info->cursor_char_width = offx - prev_offx;
				render_info->cursor_line = render_info->current_line;
				render_info->cursor_column = i;
			}
		}
		if (offx > max_width) {
			
			if (render_info && render_info->cursor_position == render_info->in_offset + i) {
				render_info->cursor_line_char_count = num_rendered;
			} else if (render_info && render_info->in_offset + i < render_info->cursor_position) {
				render_info->cursor_prev_line_char_count = num_rendered;
			}
			break;
		}

		vertex3d v[4];
		v[0] = (vertex3d) { (vec3) { xmin, ymin, 0.0f }, (vec2) { quad.s0, quad.t1 } };
		v[1] = (vertex3d) { (vec3) { xmax, ymin, 0.0f }, (vec2) { quad.s1, quad.t1 } };
		v[2] = (vertex3d) { (vec3) { xmin, ymax, 0.0f }, (vec2) { quad.s0, quad.t0 } };
		v[3] = (vertex3d) { (vec3) { xmax, ymax, 0.0f }, (vec2) { quad.s1, quad.t0 } };

		glUniform1i(glGetUniformLocation(font_rendering.font_shader, "use_texture"), 1);
		glUniform4fv(font_rendering.font_color_uniform_location, 1, (GLfloat*)color);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(v), v);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);

		// debug
		if(debug_font_rendering.font_boxes)
		{
			vec4 debug_yellow = (vec4) { 1.0f, 1.0f, 0.0f, 0.5f };

			glUniform4fv(font_rendering.font_color_uniform_location, 1, (GLfloat*)&debug_yellow);

			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);
			glBegin(GL_LINES);

			glVertex3f(x + prev_offx, y + font_rendering.ascent - font_rendering.descent, 0.0f);		// top
			glVertex3f(x + offx, y + font_rendering.ascent - font_rendering.descent, 0.0f);			//

			glVertex3f(x + prev_offx, y + font_rendering.descent, 0.0f);	// bottom
			glVertex3f(x + offx, y + font_rendering.descent, 0.0f);			//
				
			glVertex3f(x + prev_offx, y + font_rendering.ascent - font_rendering.descent, 0.0f);		// left
			glVertex3f(x + prev_offx, y + font_rendering.descent, 0.0f);	//

			glVertex3f(x + offx, y + font_rendering.ascent - font_rendering.descent, 0.0f);			// right
			glVertex3f(x + offx, y + font_rendering.descent, 0.0f);			//

			glEnd();
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
		}
	}
	glDisable(GL_BLEND);
	if (render_info && render_info->current_line == render_info->cursor_line) {
		render_info->cursor_line_char_count = num_rendered;
	}
	return num_rendered;
}

// DEBUG

void debug_toggle_font_boxes()
{
	debug_font_rendering.font_boxes = !debug_font_rendering.font_boxes;
}