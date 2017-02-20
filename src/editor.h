#ifndef HOHEX_EDITOR_H
#define HOHEX_EDITOR_H
#include "common.h"

typedef struct {
	s64 cursor;
	int line;
} Editor_State;

void render_editor();
void handle_key_down(s32 key);

#endif