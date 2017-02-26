#ifndef HOHEX_EDITOR_H
#define HOHEX_EDITOR_H
#include "common.h"
#include "util.h"
#include "math/homath.h"

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
	Text_Container container;
} Console_Info;

typedef struct {
	s64 cursor_offset;
	s64 cursor_column;
	s64 previous_line_count;
	s64 this_line_count;
	s64 next_line_count;

	bool handle_seek;
	vec2 seek_position;
} Cursor_Info;

typedef struct {
	Text_Container container;
	Cursor_Info cursor_info;
	s64 buffer_size;
	u8* buffer;
	bool render;
	bool debug;
	bool console_active;
	Editor_Mode mode;
	Console_Info console_info;
} Editor_State;

void render_editor_ascii_mode();
void update_container(Text_Container* container);

void handle_key_down(s32 key);
void handle_lmouse_down(int x, int y);
void editor_insert_text(char c);
#endif
