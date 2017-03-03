#ifndef HOHEX_INPUT
#define HOHEX_INPUT

#define MAX_KEYS 1024

#define SHIFT_KEY 16
#define CTRL_KEY 17
#define BACKSPACE_KEY 8
#define CARRIAGE_RETURN_KEY 13

typedef struct {
	bool key[MAX_KEYS];
} Keyboard_State;

extern Keyboard_State keyboard_state;

#endif
