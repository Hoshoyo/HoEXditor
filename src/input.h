#ifndef HOHEX_INPUT
#define HOHEX_INPUT

#define MAX_KEYS 1024

typedef struct {
	bool key[MAX_KEYS];
} Keyboard_State;

extern Keyboard_State keyboard_state;

#endif
