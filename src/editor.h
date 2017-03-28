#ifndef HOHEX_EDITOR_H
#define HOHEX_EDITOR_H
#include "common.h"
#include "util.h"
#include "math/homath.h"
#include "text.h"

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
typedef struct interface_panel_struct interface_panel;

struct Editor_State_s {
	Text_Container container;
	Cursor_Info cursor_info;

	text_id main_buffer_tid;
	s64 buffer_valid_bytes;
	u8* buffer;

	s32 (*individual_char_handler)(Editor_State*, s32);

	vec4 cursor_color;
	vec4 font_color;
	vec4 line_number_color;

	s64 last_line_count;
	s64 first_line_count;
	s64 first_line_number;
	bool render;
	bool debug;
	bool selecting;
	bool line_wrap;
	bool update_line_number;
	bool render_line_numbers;
	bool show_cursor;
	bool exited_on_limit_height;

	Editor_Mode mode;

	interface_panel* parent_panel;
};

void init_editor_state(Editor_State* es);

void setup_view_buffer(Editor_State* es, s64 offset, s64 size, bool force_loading);
void update_and_render_editor(Editor_State* es);
void update_container(Editor_State* es);
void update_buffer(Editor_State* es);

void editor_handle_command();
Editor_Mode next_mode(Editor_State* es);

void cursor_force(Editor_State* es, s64 pos);
void cursor_left(Editor_State* es, s64 decr);
void cursor_right(Editor_State* es, s64 incr);
void cursor_down(Editor_State* es, s64 incr);
void cursor_up(Editor_State* es, s64 incr);
void cursor_home(Editor_State* es, s64 incr);
void cursor_end(Editor_State* es, s64 incr);
void cursor_change_by_click(Editor_State* es, int x, int y);
void editor_end_selection(Editor_State* es);
void editor_start_selection(Editor_State* es);
void editor_reset_selection(Editor_State* es);
void editor_select_all(Editor_State* es);
void editor_force_selection(Editor_State* es, s64 cursor_begin, s64 cursor_end);
#endif
