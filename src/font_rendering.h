#ifndef HOHEX_FONT_RENDERING_H
#define HOHEX_FONT_RENDERING_H

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
	quad q;
	mat4 projection;
	s8 atlas_bitmap[ATLAS_SIZE * ATLAS_SIZE];
	stbtt_packedchar packedchar[LAST_CHAR];

	stbtt_fontinfo font_info;
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

typedef struct {
	int cursor_position;
	float advance_x_cursor;
} Font_Render_Info;

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

// Updates the orthographic matrix for the current font rendering context, width and height
// are the current window width and height
void update_font(float width, float height);

// Renders text on the specified positions x, y on the screen, coordinates are given in pixels
// text is the ascii encoding for the text to be rendered and color is a vec4 RGBA.
int render_text(float x, float y, u8* text, s32 length, float max_width, vec4* color, Font_Render_Info* render_info);

void render_transparent_quad(float minx, float miny, float maxx, float maxy, vec4* color);

// DEBUG
void debug_toggle_font_boxes();

#endif // HOHEX_FONT_RENDERING_H