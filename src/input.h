#ifndef HOHEX_INPUT
#define HOHEX_INPUT

#include "common.h"

#define MAX_KEYS 1024

#define SHIFT_KEY 16
#define CTRL_KEY 17
#define BACKSPACE_KEY 8
#define LINE_FEED_KEY 10
#define CARRIAGE_RETURN_KEY 13

typedef struct {
	s32 x, y;
	s32 x_left, y_left;
	s32 wheel_value;

	bool left_down;
	bool right_down;
	bool middle_down;

	bool is_captured;
} Mouse_State;

typedef struct {
	bool key[MAX_KEYS];
} Keyboard_State;

extern Keyboard_State keyboard_state;
extern Mouse_State mouse_state;

void handle_key_down(s32 key, s32 mod);
void handle_char_down(s32 key);
void handle_key_up(s32 key);
void handle_mouse_move(s32 x, s32 y);
void handle_lmouse_down(s32 x, s32 y);
void handle_rmouse_down(s32 x, s32 y);
void handle_file_drop(s32 x, s32 y, u8* path);

#endif
