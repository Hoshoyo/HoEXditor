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

typedef struct {
	s64 cursor;
	s64 buffer_size;
	u8* buffer;
	s32 line;
	Text_Container container;
} Editor_State;

void render_editor();
void update_container(Text_Container* container);

void handle_key_down(s32 key);
#endif