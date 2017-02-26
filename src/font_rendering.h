#ifndef HOHEX_FONT_RENDERING_H
#define HOHEX_FONT_RENDERING_H
#include "common.h"

#include "stb_rect_pack.h"
#include "stb_truetype.h"
#include "util.h"
#include "math/homath.h"
#include "ho_gl.h"

#define ATLAS_SIZE 2096
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
	GLuint atlas_texture_uniform_location;
	GLuint font_color_uniform_location;
	GLuint projection_uniform_location;

	GLuint attrib_pos_loc;
	GLuint attrib_texcoord_loc;

	GLuint atlas_texture;
	GLuint font_shader;
	mat4 projection;
	quad q;

	s8 atlas_bitmap[ATLAS_SIZE * ATLAS_SIZE];
	stbtt_packedchar packedchar[LAST_CHAR];
	float codepoint_width[LAST_CHAR];
	bool glyph_exists[LAST_CHAR];

	stbtt_fontinfo font_info;
	u8* ttf_buffer;
	int font_size;
	float downsize;

	float max_height;
	float max_width;
	float ascent;
	float descent;
} Font_Rendering;

typedef struct {
	bool font_boxes;
} Debug_Font_Rendering;

internal const render_info_exit_on_line_feed = FLAG(0);
internal const render_info_exit_on_carr_return = FLAG(1);
internal const render_info_exited_on_line_feed = FLAG(2);
internal const render_info_exited_on_carr_return = FLAG(3);
//const render_info_ignore_carr_return = FLAG(4);

typedef struct {
	s64 in_offset;			// this needs to be set by the caller to the position offset of the text rendered in the current buffer	
	s64 cursor_position;	// this needs to be set by the caller, cursor position of the buffer
	float advance_x_cursor;
	float cursor_char_width;
	float last_x;
	s64 current_line;	// in
	s64 cursor_line;	// out
	s64 cursor_column;
	s64 cursor_line_char_count;
	s64 cursor_prev_line_char_count;
	u32 flags;
} Font_Render_Info;

typedef struct {
	bool exited_on_limit_width;
	float exit_width;
	float excess_width;
	s32 num_chars_rendered;
	float cursor_minx;		// only set if cursor_offset != -1
	float cursor_maxx;		// only set if cursor_offset != -1
} Font_RenderOutInfo;

typedef struct {
	bool exit_on_max_width;
	float max_width;
	s64 cursor_offset;		// this must be -1 if the caller doesnt want it to be considered
} Font_RenderInInfo;

extern Font_Rendering font_rendering;
extern Debug_Font_Rendering debug_font_rendering;

// Recompiles the shader that renders fonts
//  - returns 0 on fails and only warns through logging
//  - returns 1 on success
int recompile_font_shader();

// initialized the texture atlas and data necessary to render fonts, also calculates size of ascent
// and descent and max height of the font depending on its font_size; win_width and win_height are
// the windows width and height necessary to initialize the orthographic matrix for rendering.
// On fail the function aborts the execution
void init_font(u8* filename, s32 font_size, float win_width, float win_height);
// deletes memory allocated for font in opengl and the font file
void release_font();
// loads the font using stb_truetype and created a texture for it
// when loading again, font_rendering.atlas texture must be deleted
// and ttf_buffer must be freed
void load_font(u8* filename, s32 font_size);

// Updates the orthographic matrix for the current font rendering context, width and height
// are the current window width and height
void update_font(float width, float height);

// Renders text on the specified positions x, y on the screen, coordinates are given in pixels
// text is the ascii encoding for the text to be rendered and color is a vec4 RGBA.
int render_text(float x, float y, u8* text, s32 length, float max_width, vec4* color, Font_Render_Info* render_info);
// @refactor this into the same render_text, make a more usable api
//int render_text2(float x, float y, u8* text, s32 length, float max_width, vec4* color, Font_Render_Info* render_info);
int render_text2(float x, float y, u8* text, s32 length, vec4* color);

void render_transparent_quad(float minx, float miny, float maxx, float maxy, vec4* color);

int prerender_text(float x, float y, u8* text, s32 length, Font_RenderOutInfo* out_info, Font_RenderInInfo* in_info);

// DEBUG
void debug_toggle_font_boxes();

#endif // HOHEX_FONT_RENDERING_H