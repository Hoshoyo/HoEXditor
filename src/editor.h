#ifndef HOHEX_EDITOR_H
#define HOHEX_EDITOR_H
#include "common.h"
#include "util.h"

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
	s64 cursor;
	s64 cursor_column;
	s64 cursor_line_char_count;
	s64 cursor_prev_line_char_count;
	s64 buffer_size;
	u8* buffer;
	Text_Container container;
	bool render;
	bool debug;
	Editor_Mode mode;
} Editor_State;

void render_editor_ascii_mode();
void update_container(Text_Container* container);

void handle_key_down(s32 key);
void insert_text_test(char c);
#endif
