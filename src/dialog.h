#ifndef HOHEX_DIALOG_H
#define HOHEX_DIALOG_H

#include "interface.h"

#define MAX_NUM_DIALOGS 16
#define UI_DIALOG_VIEW_BUFFER_SIZE 1024
#define UI_DIALOG_INPUT_BUFFER_SIZE 1024

ui_dialog* create_dialog
	(float x,
	float y,
	float height,
	float width,
	float ratio,
	vec4 view_font_color,
	vec4 view_cursor_color,
	vec4 view_background_color,
	vec4 input_font_color,
	vec4 input_cursor_color,
	vec4 input_background_color,
	bool dark_background_when_rendering,
	void(*dialog_callback)(u8*));
void change_view_content(ui_dialog* dialog, u8* text, u64 text_size);
void change_dialog_visibility(ui_dialog* dialog, bool visible);
void destroy_dialog(ui_dialog* dialog);
void update_dialogs();
void render_dialogs();
s32 dialog_char_handler(Editor_State* es, s32 key);

#endif