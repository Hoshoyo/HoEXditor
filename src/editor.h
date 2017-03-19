#ifndef HOHEX_EDITOR_H
#define HOHEX_EDITOR_H
#include "common.h"
#include "util.h"
#include "math/homath.h"

#if HACKER_THEME
#define FONT_COLOR (vec4) { 0.0f, 0.9f, 0.0f, 1.0f }
#define CURSOR_COLOR (vec4) { 1.0f, 1.0f, 1.0f, 0.5f }
#elif WHITE_THEME
#define FONT_COLOR (vec4) { 0.1f, 0.1f, 0.1f, 1.0f }
#define CURSOR_COLOR (vec4) { 0.0f, 0.0f, 0.0f, 0.8f }
#else
#define CURSOR_COLOR (vec4) { 0.0f, 0.48f, 0.8f, 1.0f }
#define FONT_COLOR (vec4) { 0.9f, 0.9f, 0.9f, 1.0f }
#endif

#define MAX_EDITORS 8

typedef struct {
	float minx;
	float miny;
	float maxx;
	float maxy;

	float left_padding;
	float right_padding;
	float bottom_padding;
	float top_padding;
} Text_Container;

typedef enum {
	EDITOR_MODE_ASCII	= 0,
	EDITOR_MODE_HEX		= 1,
	EDITOR_MODE_BINARY	= 2,
	EDITOR_MODE_END		= 3,
} Editor_Mode;

typedef struct {
	s64 block_offset;

	s64 selection_offset;
	s64 cursor_offset;
	s64 cursor_column;
	s64 cursor_line;
	s64 last_line;
	s64 cursor_snaped_column;
	s64 previous_line_count;
	s64 this_line_count;
	s64 next_line_count;

	bool handle_seek;
	vec2 seek_position;
} Cursor_Info;

typedef struct Editor_State_s Editor_State;

typedef struct Console_Info_s {
	Text_Container container;
	bool console_active;
	Editor_State* linked_console;
} Console_Info;

struct Editor_State_s {
	Text_Container container;
	Cursor_Info cursor_info;

	s32 main_buffer_id;
	s64 buffer_size;
	s64 buffer_valid_bytes;
	u8* buffer;

	s64 last_line_count;
	s64 first_line_count;
	s64 first_line_number;
	bool render;
	bool debug;
	bool selecting;
	bool line_wrap;
	bool update_line_number;
	bool render_line_numbers;
	bool is_block_text;

	Editor_Mode mode;

	Console_Info console_info;

	vec4 cursor_color;
	vec4 font_color;
};

Editor_State** init_editor();
void render_editor(Editor_State* es);
void render_editor_ascii_mode();
void update_container(Editor_State* es);
void update_buffer();

void handle_key_down(s32 key);
void handle_lmouse_down(int x, int y);
void editor_end_selection();
void editor_start_selection();
void editor_reset_selection();
#endif
