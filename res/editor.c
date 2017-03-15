#include "editor.h"
#include "math/homath.h"
#include "font_rendering.h"
#include "text_manager.h"
#include "Psapi.h"
#include "memory.h"
#include "input.h"
#include "text_events.h"
#include "interface.h"
#include "os_dependent.h"
#include "interface.h"

#define DEBUG 0

Editor_State editor_state = {0};

extern Window_State win_state;

#define INIT_TEXT_CONTAINER(Cont, MINX, MAXX, MINY, MAXY, LP, RP, TP, BP) \
Cont.minx = MINX;	\
Cont.maxx = MAXX;	\
Cont.miny = MINY;	\
Cont.maxy = MAXY;	\
Cont.left_padding = LP;	\
Cont.right_padding = RP; \
Cont.top_padding = TP;	\
Cont.bottom_padding = BP	\

GLuint my_texture;
int w, h, c;

void init_editor()
{
	//char font[] = "res/LiberationMono-Regular.ttf";
	//char font[] = "c:/windows/fonts/times.ttf";
	char font[] = "c:/windows/fonts/consola.ttf";
	s32 font_size = 16;	// @TEMPORARY @TODO make this configurable
	init_font(font, font_size, win_state.win_width, win_state.win_height);
	init_interface();

	//load_file("./res/empty.txt");
	//load_file("./res/cedilha");	// @temporary, init this in the proper way
	load_file("./res/m79.txt");

	//save_file("./res/haha.txt");

	u8 word_to_search[256] = "Buddha";
	ho_search_result* result = search_word(0, _tm_text_size - 1, word_to_search, hstrlen(word_to_search));

	print("SEARCH RESULTS:\n");
	u32 num_results = 0;
	while (result != null)
	{
		print("%d. %d\n", ++num_results, result->cursor_position);
		void* last = result;
		result = result->next;
		hfree(last);
	}

	// init cursor state
	editor_state.cursor_info.cursor_offset = 0;
	editor_state.cursor_info.cursor_column = 0;
	editor_state.cursor_info.cursor_snaped_column = 0;
	editor_state.cursor_info.previous_line_count = 0;
	editor_state.cursor_info.next_line_count = 0;
	editor_state.cursor_info.this_line_count = 0;
	editor_state.cursor_info.cursor_line = 0;

	editor_state.buffer_size = _tm_text_size;
	editor_state.buffer = get_text_buffer(4096, 0);

	editor_state.console_active = false;
	editor_state.render = true;
	editor_state.debug = true;
	editor_state.mode = EDITOR_MODE_ASCII;

	editor_state.cursor_info.handle_seek = false;

	INIT_TEXT_CONTAINER(editor_state.console_info.container, 0.0f, win_state.win_width, 0.0f, MIN(200.0f, win_state.win_height / 2.0f), 0.0f, 0.0f, 0.0f, 0.0f);

	// @temporary initialization of container for the editor
	INIT_TEXT_CONTAINER(editor_state.container, 0.0f, 0.0f, 0.0f, 0.0f, 20.0f, 200.0f, 2.0f + font_rendering->max_height, 20.0f);
	ui_update_text_container_paddings(&editor_state.container);
	update_container(&editor_state.container);
}

void update_container(Text_Container* container)
{
	// this has to change the state of the editor because when the window is minimized
	// it cannot render
	container->minx = container->left_padding;
	container->maxx = MAX(0.0f, win_state.win_width - container->right_padding);
	container->miny = container->bottom_padding;
	container->maxy = MAX(0.0f, win_state.win_height - container->top_padding);
	if (win_state.win_width == 0 || win_state.win_height == 0 || container->maxx == 0 || container->maxy == 0) {
		editor_state.render = false;
	} else if(win_state.win_width > 0 && win_state.win_height > 0) {
		editor_state.render = true;
	}
}

internal void render_debug_info()
{

}

internal void render_selection(int num_lines, int num_bytes, int line_written, Font_RenderOutInfo* out_info) {
	float min_y = editor_state.container.maxy - ((font_rendering->max_height) * (float)(num_lines - 1)) + font_rendering->descent;
	float max_y = editor_state.container.maxy - ((font_rendering->max_height) * (float)(num_lines - 2)) + font_rendering->descent;
	float max_x = 0.0f;
	float min_x = 0.0f;

	// if the selection is happening with the selection cursor back
	if (editor_state.cursor_info.selection_offset < editor_state.cursor_info.cursor_offset) {
		int line_count = editor_state.cursor_info.cursor_offset - (num_bytes - line_written);
		int selec_count = editor_state.cursor_info.selection_offset - (num_bytes - line_written);
		bool is_cursor_in_this_line = (line_count < line_written && line_count >= 0) ? true : false;
		bool is_selection_in_this_line = (selec_count < line_written && selec_count >= 0) ? true : false;
		if (is_selection_in_this_line) {
			if (is_cursor_in_this_line) {
				min_x = out_info->selection_maxx;
				max_x = out_info->cursor_minx;
			} else {
				min_x = out_info->selection_maxx;
				max_x = out_info->exit_width;
			}
		} else if (num_bytes - line_written >= editor_state.cursor_info.selection_offset &&
			num_bytes <= editor_state.cursor_info.cursor_offset) {
			mi== EDITOR_MODE_HEX) ? 0 : snap;
		int c = editor_state.cursor_info.previous_line_count - MAX(snap, editor_state.cursor_info.cursor_column - 1);
		if (c <= 0) c = 1;
		c += editor_state.cursor_info.cursor_column;

		if (editor_state.cursor_info.cursor_line > 1) {
			editor_state.cursor_info.cursor_offset -= c;
		}
	}
	if (key == VK_DOWN) {
		// selection_stuff
		if (keyboard_state.key[VK_SHIFT]) editor_start_selection();
		else if (selection_reset) return;

		if (editor_state.selecting && editor_state.cursor_info.cursor_offset == editor_state.cursor_info.selection_offset) {
			editor_state.cursor_info.selection_offset = editor_state.cursor_info.cursor_offset;
		}

		int c = editor_state.cursor_info.this_line_count - editor_state.cursor_info.cursor_column;
		int after_cursor_line_count = c;
		c += editor_state.cursor_info.cursor_column;
		int next_line_c = MIN(editor_state.cursor_info.next_line_count - 1 + after_cursor_line_count,
							  MAX(c, editor_state.cursor_info.cursor_snaped_column + 1));

		if (editor_state.cursor_info.cursor_offset + next_line_c < editor_state.buffer_size &&
			editor_state.cursor_info.next_line_count > 0) {
			editor_state.cursor_info.cursor_offset += next_line_c;
		}
	}

	if (key == VK_HOME) {
		editor_state.cursor_info.cursor_offset -= editor_state.cursor_info.cursor_column;
	}
	if (key == VK_END) {
		editor_state.cursor_info.cursor_offset += editor_state.cursor_info.this_line_count - editor_state.cursor_info.cursor_column - 1;
	}

	if (editor_state.cursor_info.cursor_offset != cursor && editor_state.cursor_info.cursor_offset < editor_state.buffer_size) {
		//set_cursor_begin(editor_state.cursor_info.cursor_offset);
	}
}

void handle_lmouse_down(int x, int y)
{
	float xf = (float)x;
	float yf = (float)y;

	yf = win_state.win_height - yf;

	editor_state.cursor_info.handle_seek = true;
	editor_state.cursor_info.seek_position.x = xf;
	editor_state.cursor_info.seek_position.y = yf;
}
