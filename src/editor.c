#include "editor.h"
#include "math/homath.h"
#include "font_rendering.h"
#include "text_manager.h"
#include "Psapi.h"
#include "memory.h"
#include "input.h"
#include "text_events.h"

#define DEBUG 0

Editor_State editor_state = {0};

typedef struct {
	HWND window_handle;
	LONG win_width, win_height;
	WINDOWPLACEMENT g_wpPrev;
	HDC device_context;
	HGLRC rendering_context;
} Window_State;

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

void init_editor()
{
	//char font[] = "res/LiberationMono-Regular.ttf";
	//char font[] = "c:/windows/fonts/times.ttf";
	char font[] = "c:/windows/fonts/consola.ttf";
	s32 font_size = 16;	// @TEMPORARY @TODO make this configurable
	init_font(font, font_size, win_state.win_width, win_state.win_height);

	load_file("./res/dummy.txt");	// @temporary, init this in the proper way
	load_file("./res/m79.txt");

	save_file("./res/haha.txt");

	// init cursor state
	editor_state.cursor_info.cursor_offset = 0;
	editor_state.cursor_info.cursor_column = 0;
	editor_state.cursor_info.previous_line_count = 0;
	editor_state.cursor_info.next_line_count = 0;
	editor_state.cursor_info.this_line_count = 0;

	editor_state.buffer_size = _tm_text_size;
	editor_state.buffer = get_text_buffer(4096, 0);

	editor_state.console_active = false;
	editor_state.render = true;
	editor_state.debug = true;
	editor_state.mode = EDITOR_MODE_ASCII;

	editor_state.cursor_info.handle_seek = false;

	INIT_TEXT_CONTAINER(editor_state.console_info.container, 0.0f, win_state.win_width, 0.0f, MIN(200.0f, win_state.win_height / 2.0f), 0.0f, 0.0f, 0.0f, 0.0f);

	// @temporary initialization of container for the editor
	INIT_TEXT_CONTAINER(editor_state.container, 0.0f, 0.0f, 0.0f, 0.0f, 20.0f, 200.0f, 2.0f + font_rendering.max_height, 20.0f);
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

internal void render_editor_hex_mode()
{
	vec4 bg_color = (vec4) { 0.05f, 0.05f, 0.05f, 1.0f };
	render_transparent_quad(
		editor_state.container.minx,
		editor_state.container.miny,
		editor_state.container.maxx + editor_state.container.left_padding,
		editor_state.container.maxy,
		&bg_color);

	glEnable(GL_SCISSOR_TEST);
	glScissor(editor_state.container.minx, editor_state.container.miny, editor_state.container.maxx, editor_state.container.maxy);

	// render text in the buffer
	if (editor_state.render) {
		vec4 font_color = (vec4) { 0.8f, 0.8f, 0.8f, 1.0f };

		// Setup the rendering info needed to render hex
		Font_RenderInInfo in_info = { 0 };
		Font_RenderOutInfo out_info = { 0 };
		{
			in_info.cursor_offset = -1;
			in_info.exit_on_max_width = true;
			in_info.max_width = editor_state.container.maxx;
			in_info.seek_location = false;

			if (editor_state.cursor_info.handle_seek) {
				in_info.location_to_seek.x = editor_state.cursor_info.seek_position.x;
				in_info.location_to_seek.y = editor_state.cursor_info.seek_position.y;
				in_info.seek_location = true;
				editor_state.cursor_info.handle_seek = false;
			}
		}

		const float left_text_padding = 20.0f;	// in pixels
		const float spacing = 4.0f;				// in pixels

		float offset_y = 0, offset_x = left_text_padding;
		int written = 0, num_bytes = 0, num_lines = 1, cursor_line = 0;
		int last_line_count = 0;
		int line_count = 0;

		while (num_bytes < editor_state.buffer_size) {
			char hexbuffer[64];
			u64 num = *(editor_state.buffer + num_bytes);
			int num_len = u8_to_str_base16(num, false, hexbuffer);

			if (num_bytes == editor_state.cursor_info.cursor_offset) {
				in_info.cursor_offset = 0;
				cursor_line = num_lines;
			} else {
				in_info.cursor_offset = -1;
			}

			written = prerender_text(editor_state.container.minx + offset_x, editor_state.container.maxy - font_rendering.max_height + offset_y,
				hexbuffer, num_len, &out_info, &in_info);

			// test seeking cursor from click
			if (out_info.seeked_index != -1) {
				editor_state.cursor_info.cursor_offset = num_bytes;
			}

			if (out_info.exited_on_limit_width) {
				// set the count of characters rendered from the cursor previous, current and next lines
				if (cursor_line == num_lines) {
					editor_state.cursor_info.this_line_count = line_count;
					editor_state.cursor_info.previous_line_count = last_line_count;
				}
				if (cursor_line + 1 == num_lines) {
					editor_state.cursor_info.next_line_count = line_count;
				}

				offset_y -= font_rendering.max_height;
				offset_x = left_text_padding;
				num_lines++;
				last_line_count = line_count;
				line_count = 0;
				continue;
			} else {
				line_count += written / 2;
				render_text(editor_state.container.minx + offset_x, editor_state.container.maxy - font_rendering.max_height + offset_y,
					hexbuffer, num_len, &font_color);
				offset_x = (out_info.exit_width - editor_state.container.minx) + spacing;
			}

			num_bytes++;
			if (written == 0) break;	// if the space to render is too small for a single character than just leave
		}
		// render cursor overtop
		vec4 cursor_color = (vec4) { 0.5f, 0.9f, 0.85f, 0.5f };
		float min_y = editor_state.container.maxy - ((font_rendering.max_height) * (float)cursor_line) + font_rendering.descent;
		float max_y = editor_state.container.maxy - ((font_rendering.max_height) * (float)(cursor_line - 1)) + font_rendering.descent;
		render_transparent_quad(out_info.cursor_minx, min_y, out_info.cursor_maxx, max_y, &cursor_color);

		glDisable(GL_SCISSOR_TEST);
	}
}

internal void render_editor_ascii_mode()
{
	glEnable(GL_SCISSOR_TEST);
	glScissor(editor_state.container.minx, editor_state.container.miny, editor_state.container.maxx, editor_state.container.maxy);

	if (editor_state.render) {
		vec4 font_color = (vec4) { 0.8f, 0.8f, 0.8f, 1.0f };

		Font_RenderInInfo in_info = { 0 };
		Font_RenderOutInfo out_info = { 0 };

		{
			in_info.cursor_offset = -1;
			in_info.exit_on_max_width = true;
			in_info.max_width = editor_state.container.maxx;
			in_info.exit_on_line_feed = true;
			in_info.seek_location = false;
			in_info.selection_offset = -1;
			editor_state.cursor_info.this_line_count = -1;
			editor_state.cursor_info.previous_line_count = -1;
			editor_state.cursor_info.next_line_count = -1;

			if (editor_state.cursor_info.handle_seek) {
				in_info.location_to_seek.x = editor_state.cursor_info.seek_position.x;
				in_info.location_to_seek.y = editor_state.cursor_info.seek_position.y;
				in_info.seek_location = true;
				editor_state.cursor_info.handle_seek = false;
			}
		}

		int written = 0, num_bytes = 0, cursor_line = 0, num_lines = 1, selection_line = 0;
		float offset_y = 0, offset_x = 0;
		int last_line_count = 0;

		while (num_bytes < _tm_valid_bytes) {

			if (num_bytes <= editor_state.cursor_info.cursor_offset) {
				in_info.cursor_offset = editor_state.cursor_info.cursor_offset - num_bytes;
				cursor_line = num_lines;
			} else {
				in_info.cursor_offset = -1;
			}

			if (num_bytes <= editor_state.cursor_info.selection_offset &&
				editor_state.cursor_info.selection_offset != editor_state.cursor_info.cursor_offset) {
				in_info.selection_offset = editor_state.cursor_info.selection_offset - num_bytes;
				selection_line = num_lines;
			} else {
				in_info.selection_offset = -1;
			}

			s64 num_to_write = editor_state.buffer_size - num_bytes - (editor_state.buffer_size - _tm_valid_bytes);
			written = prerender_text(editor_state.container.minx, editor_state.container.maxy - font_rendering.max_height + offset_y,
				editor_state.buffer + num_bytes, num_to_write, &out_info, &in_info);

			// test seeking cursor from click
			if (out_info.seeked_index != -1) {
				editor_state.cursor_info.cursor_offset = num_bytes + out_info.seeked_index;
			}

			written = render_text(editor_state.container.minx, editor_state.container.maxy - font_rendering.max_height + offset_y,
				editor_state.buffer + num_bytes, written, &font_color);

			// set the count of characters rendered from the cursor previous, current and next lines
			if (cursor_line == num_lines) {
				editor_state.cursor_info.this_line_count = written;
				editor_state.cursor_info.previous_line_count = last_line_count;
			}
			if (cursor_line + 1 == num_lines) {
				editor_state.cursor_info.next_line_count = written;
			}

			if (out_info.exited_on_limit_width || out_info.exited_on_line_feed) {
				offset_y -= font_rendering.max_height;
				offset_x = 0.0f;
				num_lines++;
				last_line_count = written;
			}

			num_bytes += written;
			if (written == 0) break;	// if the space to render is too small for a single character than just leave
		}

		// render cursor overtop
		vec4 cursor_color = (vec4) { 0.5f, 0.9f, 0.85f, 0.5f };
		float min_y = editor_state.container.maxy - ((font_rendering.max_height) * (float)cursor_line) + font_rendering.descent;
		float max_y = editor_state.container.maxy - ((font_rendering.max_height) * (float)(cursor_line - 1)) + font_rendering.descent;
		render_transparent_quad(out_info.cursor_minx, min_y, out_info.cursor_maxx, max_y, &cursor_color);

		// selection
		if (editor_state.selecting && editor_state.cursor_info.selection_offset != editor_state.cursor_info.cursor_offset) {
			float min_selec_y = editor_state.container.maxy - ((font_rendering.max_height) * (float)selection_line) + font_rendering.descent;
			float max_selec_y = editor_state.container.maxy - ((font_rendering.max_height) * (float)(selection_line - 1)) + font_rendering.descent;
			render_transparent_quad(out_info.selection_minx, min_selec_y, out_info.cursor_maxx, max_selec_y, &cursor_color);
		}
	}
	glDisable(GL_SCISSOR_TEST);
}

void render_console()
{
	vec4 console_bg_color = (vec4) { 0.0f, 0.0f, 0.1f, 0.8f };
	render_transparent_quad(
		editor_state.console_info.container.minx,
		editor_state.console_info.container.miny,
		editor_state.console_info.container.maxx,
		editor_state.console_info.container.maxy,
		&console_bg_color);
}

void editor_start_selection() {
	if (!editor_state.selecting) {
		editor_state.cursor_info.selection_offset = editor_state.cursor_info.cursor_offset;
	}
	editor_state.selecting = true;
}

void editor_end_selection() {
	editor_state.selecting = false;
}

void render_editor()
{
	editor_state.buffer_size = MIN(4096, _tm_text_size);
	update_container(&editor_state.container);

	switch (editor_state.mode) {
	case EDITOR_MODE_ASCII: {
		render_editor_ascii_mode();
	}break;
	case EDITOR_MODE_HEX: {
		render_editor_hex_mode();
	}break;
	}

	if (editor_state.debug) {
		//render_debug_info();
	}
	if (editor_state.console_active) {
		render_console();
	}
}

internal Editor_Mode next_mode() {
	int mode = editor_state.mode;
	mode++;
	if (mode >= EDITOR_MODE_END) mode = 0;
	return mode;
}

void handle_key_down(s32 key)
{
	if (key != VK_SHIFT && !keyboard_state.key[VK_SHIFT]) {
		editor_state.selecting = false;
	}

	s64 cursor = editor_state.cursor_info.cursor_offset;

	if (key == 'R' && keyboard_state.key[17]) { recompile_font_shader(); return; }
	if (key == 'F' && keyboard_state.key[17]) { debug_toggle_font_boxes(); return; }
	if (key == 'P' && keyboard_state.key[17]) {	editor_state.mode = next_mode(); }
	if (key == VK_F1) {
		editor_state.console_active = !editor_state.console_active;
	}

	if (key == VK_RIGHT) {
		s64 increment = 1;
		if (keyboard_state.key[17]) increment = 8;

		if (keyboard_state.key[VK_SHIFT] && editor_state.cursor_info.cursor_offset == editor_state.cursor_info.selection_offset) {
			editor_state.cursor_info.selection_offset = editor_state.cursor_info.cursor_offset;
		}
		editor_state.cursor_info.cursor_offset = MIN(editor_state.cursor_info.cursor_offset + increment, editor_state.buffer_size);
	}
	if (key == VK_LEFT) {
		s64 increment = 1;
		if (keyboard_state.key[17]) increment = 8;

		if (keyboard_state.key[VK_SHIFT] && editor_state.cursor_info.cursor_offset == editor_state.cursor_info.selection_offset) {
			editor_state.cursor_info.selection_offset = editor_state.cursor_info.cursor_offset;
		}
		editor_state.cursor_info.cursor_offset = MAX(editor_state.cursor_info.cursor_offset - increment, 0);
	}

	if (key == VK_UP) {
		int c = editor_state.cursor_info.previous_line_count;
		if (keyboard_state.key[VK_SHIFT] && editor_state.cursor_info.cursor_offset == editor_state.cursor_info.selection_offset) {
			editor_state.cursor_info.selection_offset = editor_state.cursor_info.cursor_offset;
		}

		if (editor_state.cursor_info.cursor_offset - c > 0) {
			editor_state.cursor_info.cursor_offset -= c;
		}
	}
	if (key == VK_DOWN) {
		int c = editor_state.cursor_info.this_line_count - editor_state.cursor_info.cursor_column;
		c += editor_state.cursor_info.cursor_column;

		if (keyboard_state.key[VK_SHIFT] && editor_state.cursor_info.cursor_offset == editor_state.cursor_info.selection_offset) {
			editor_state.cursor_info.selection_offset = editor_state.cursor_info.cursor_offset;
		}

		if (editor_state.cursor_info.cursor_offset + c < editor_state.buffer_size) {
			editor_state.cursor_info.cursor_offset += c;
		}
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

void editor_insert_text(u8 c)
{
	if (c != BACKSPACE_KEY)
	{
		u8* inserted_text = halloc(sizeof(u8));
		*inserted_text = c;

		insert_text(inserted_text, 1, editor_state.cursor_info.cursor_offset);
		add_undo_item(HO_INSERT_TEXT, inserted_text, sizeof(u8), editor_state.cursor_info.cursor_offset);

		editor_state.cursor_info.cursor_offset += 1;
	}
	else
	{
		if (editor_state.cursor_info.cursor_offset > 0)
		{
			u8* deleted_text = halloc(sizeof(u8));

			delete_text(deleted_text, 1, editor_state.cursor_info.cursor_offset - 1);
			add_undo_item(HO_DELETE_TEXT, deleted_text, sizeof(u8), editor_state.cursor_info.cursor_offset - 1);

			editor_state.cursor_info.cursor_offset -= 1;
		}
	}

	check_text();
	check_arenas();
}
