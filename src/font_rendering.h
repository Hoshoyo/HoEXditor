#pragma once
#include "common.h"

#undef internal
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#define internal static

typedef int Font_ID;

struct Character {
	hm::vec2 botl, botr, topl, topr;
	s32 size[2];
	s32 bearing[2];
	GLuint advance;
	bool renderable;
};

const u32 FONT_ASCII_ONLY = FLAG(0);
const u32 FONT_UTF8       = FLAG(1);

#define MAX_UNICODE 65536

struct Font_Info {
	string name;

	s64 font_size;
	s64 atlas_size;
	s64 max_height;
	s64 max_width;

	s64 text_buffer_length;
	s64 text_buffer_max_length;
	s64 text_buffer_offset;

	u32 flags = 0;

	u32 shader = -1;
	u32 atlas_full_id = -1;
	u32 atlas_asci_id = -1;

	u32 vao;
	u32 vbo;
	u32 ebo;

	s32 window_width;
	s32 window_height;

	u8* atlas_data = 0;

	bool error_loading = false;
	bool loaded = false;
	bool finish_load = false;
	bool kerning = false;

	FT_Face face;
	Character characters[MAX_UNICODE];
};

string font_vshader = MAKE_STRING(R"(
	#version 330 core
	layout(location = 0) in vec3 vertex;
	layout(location = 1) in vec2 tcoords;
	layout(location = 2) in vec4 v_color;

	out vec2 texcoords;
	out vec4 out_color;

	uniform mat4 projection = mat4(1.0);

	void main(){
		gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
		texcoords = tcoords;
		out_color = v_color;
	}
)");

string font_fshader = MAKE_STRING(R"(
	#version 330 core
	in vec2 texcoords;
	in vec4 out_color;
	out vec4 color;

	uniform sampler2D text;

	void main(){
		vec4 sampled = texture(text, texcoords);
		color = vec4(out_color.rgb, sampled.r * out_color.a);
		//color = vec4(1.0, 1.0, 1.0, 1.0);
		//color = out_color;
		//color = sampled;
	}
)");

u32 utf8_to_unicode(u8* text, u32* advance);
int font_load(Font_Info* font, const s8* filepath, u32 pixel_point, u32 load_limit);