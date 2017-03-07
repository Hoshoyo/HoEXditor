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
#define BATCH_SIZE 8192

#pragma pack(push)
typedef struct {
	vec3 position;
	vec2 texcoord;
} vertex3d;
#pragma pack(pop)

typedef struct {
	vertex3d v[4];
	u16 indices[6];
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
	vertex3d* vertex_data;
	u16* index_data;
	s32 queue_index;
	Font_Rendering fr;
} Batch_Font_Renderer;

typedef struct {
	bool exited_on_limit_width;
	bool exited_on_line_feed;
	
	float exit_width;
	float begin_width;
	float excess_width;
	
	s32 num_chars_rendered;

	float cursor_minx;		// only set if cursor_offset != -1
	float cursor_maxx;		// only set if cursor_offset != -1

	float selection_minx;	// only set if cursor_offset != -1
	float selection_maxx;	// only set if cursor_offset != -1
	
	float seeked_min;			// only set if seek_location == true
	float seeked_max;			// only set if seek_location == true
	int seeked_index;			// only set if seek_location == true
} Font_RenderOutInfo;

typedef struct {
	bool seek_location;
	bool exit_on_max_width;
	bool exit_on_line_feed;
	float max_width;
	s64 cursor_offset;		// this must be -1 if the caller doesnt want it to be considered
	s64 selection_offset;
	vec2 location_to_seek;
} Font_RenderInInfo;

extern Font_Rendering* font_rendering;

void bind_font(Font_Rendering** font);

// Recompiles the shader that renders fonts
//  - returns 0 on fails and only warns through logging
//  - returns 1 on success
int recompile_font_shader();

void fill_font(Font_Rendering* fr, float win_width, float win_height);

// initialized the texture atlas and data necessary to render fonts, also calculates size of ascent
// and descent and max height of the font depending on its font_size; win_width and win_height are
// the windows width and height necessary to initialize the orthographic matrix for rendering.
// On fail the function aborts the execution
void init_font(u8* filename, s32 font_size, float win_width, float win_height);

// deletes memory allocated for font in opengl and the font file
void release_font(Font_Rendering** fr);
// loads the font using stb_truetype and created a texture for it
// when loading again, font_rendering.atlas texture must be deleted
// and ttf_buffer must be freed
void load_font(u8* filename, s32 font_size, Font_Rendering** font_descriptor);

// Updates the orthographic matrix for the current font rendering context, width and height
// are the current window width and height
void update_font(float width, float height);

// Renders text on the specified positions x, y on the screen, coordinates are given in pixels
// text is the ascii encoding for the text to be rendered and color is a vec4 RGBA.
int render_text(float x, float y, u8* text, s32 length, vec4* color);

// Render a quad on the specified location, the quad can be opaque or transparent, blend will be used
void render_transparent_quad(float minx, float miny, float maxx, float maxy, vec4* color);
void render_textured_quad(float minx, float miny, float maxx, float maxy, GLuint texture_id);

GLuint gen_gl_texture(u8* texture_data, int width, int height);
u8* create_texture(u8* filename, int* width, int* height, int* channels);
void free_texture(u8* data);

// Help function to know text dimensions on screen, returns how many characters would be rendered
// depending on parameters of in_info, out_info is information needed by the caller
int prerender_text(float x, float y, u8* text, s32 length, Font_RenderOutInfo* out_info, Font_RenderInInfo* in_info);

// DEBUG
void debug_toggle_font_boxes();

// Prepare the batch renderer for up to 
void prepare_editor_text();

void queue_text(float x, float y, u8* text, s32 length);

void flush_text_batch(vec4* color, s64 num_bytes);
#endif // HOHEX_FONT_RENDERING_H