#include "editor.h"
#include "math/homath.h"
#include "font_rendering.h"
#include "text_manager.h"
#include "Psapi.h"
#include "memory.h"

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

#define MAX_KEYS 1024
typedef struct {
	bool key[MAX_KEYS];
} Keyboard_State;
extern Keyboard_State keyboard_state;

void init_editor()
{
	//char font[] = "res/LiberationMono-Regular.ttf";
	//char font[] = "c:/windows/fonts/times.ttf";
	char font[] = "c:/windows/fonts/consola.ttf";
	s32 font_size = 20;	// @TEMPORARY @TODO make this configurable
	init_font(font, font_size, win_state.win_width, win_state.win_height);

	init_text_api("./res/dummy.txt");
	//end_text_api();
	//init_text_api("./res/m79.txt");

	editor_state.cursor = 0;
	editor_state.cursor_column = 0;
	editor_state.cursor_prev_line_char_count = 0;
	editor_state.buffer_size = _tm_text_size;
	editor_state.buffer = get_text_buffer(_tm_text_size, 0);
	editor_state.render = true;
	editor_state.debug = true;
	editor_state.mode = EDITOR_MODE_ASCII;
	//end_text_api();

	Text_Container* container = &editor_state.container;
	container->left_padding = 2.0f;
	container->right_padding = 200.0f;
	container->bottom_padding = 2.0f + font_rendering.max_height;
	container->top_padding = 20.0f;
	update_container(container);
}

void update_container(Text_Container* container)
{
	// this has to change the state of the editor because when the window is minimized
	// it cannot render
	container->minx = container->left_padding;
	container->maxx = win_state.win_width - container->right_padding;
	container->miny = container->bottom_padding;
	container->maxy = win_state.win_height - container->top_padding;
	if (win_state.win_width == 0 || win_state.win_height == 0) {
		editor_state.render = false;
	} else if(win_state.win_width > 0 && win_state.win_height > 0) {
		editor_state.render = true;
	}
}

internal void render_debug_info(Font_Render_Info* in_info)
{
	vec4 font_color = (vec4) { 0.8f, 0.8f, 0.8f, 1.0f };
	Font_Render_Info render_info = { 0 };
	if(in_info) copy_mem(&render_info, in_info, sizeof(Font_Render_Info));

	PROCESS_MEMORY_COUNTERS pmcs;
	GetProcessMemoryInfo(GetCurrentProcess(), &pmcs, sizeof(pmcs));
	char buffer[64];
	int wrtn = s64_to_str_base10(pmcs.PagefileUsage, buffer);
	render_text(2.0f, -font_rendering.descent, "Memory usage: ", sizeof "Memory usage: " - 1, win_state.win_width, &font_color, &render_info);
	wrtn = render_text(render_info.last_x, -font_rendering.descent, buffer, wrtn, win_state.win_width, &font_color, &render_info);
	render_text(render_info.last_x, -font_rendering.descent, " bytes", sizeof " bytes" - 1, win_state.win_width, &font_color, &render_info);

	render_text(render_info.last_x, -font_rendering.descent, " cursor_pos: ", sizeof " cursor_pos: " - 1, win_state.win_width, &font_color, &render_info);
	wrtn = s64_to_str_base10(editor_state.cursor_column, buffer);
	wrtn = render_text(render_info.last_x, -font_rendering.descent, buffer, wrtn, win_state.win_width, &font_color, 0);
}


internal void render_editor_hex_mode()
{
	// render text in the buffer
	if (editor_state.render) {
		vec4 font_color = (vec4) { 0.8f, 0.8f, 0.8f, 1.0f };
		Font_Render_Info render_info = { 0 };
		render_info.cursor_position = editor_state.cursor;
		render_info.current_line = 0;
		int written = 0;
		float offset_y = 0, offset_x = 0;
		int num_bytes = 0;
		while (written < editor_state.buffer_size) {
			render_info.current_line++;
			render_info.in_offset = written;
			char hexbuffer[64];
			u64 num = *(editor_state.buffer + num_bytes);
			num_bytes++;
			int num_len = u8_to_str_base16(num, false, hexbuffer);

			written += render_text(editor_state.container.minx + offset_x, editor_state.container.maxy - font_rendering.max_height + offset_y,
				hexbuffer, num_len, editor_state.container.maxx, &font_color, &render_info);

			if (render_info.last_x + font_rendering.max_width >= editor_state.container.maxx) {
				offset_y -= font_rendering.max_height;
				offset_x = 0.0f;
			}
			else {
				offset_x = render_info.last_x + 4.0f;
			}

			// this hangs because when minimized the win_state.win_width and win_state.win_height is 0
			// and thus editor_state.container.maxx is negative, making render_text always return 0
			// and therefore never breaking out of the loop
			if (written == 0) break;
		}

		editor_state.cursor_column = render_info.cursor_column;
		editor_state.cursor_line_char_count = render_info.cursor_line_char_count;
		editor_state.cursor_prev_line_char_count = render_info.cursor_prev_line_char_count;
		// render cursor overtop
		vec4 cursor_color = (vec4) { 0.5f, 0.9f, 0.85f, 0.5f };

		float min_x = render_info.advance_x_cursor;
		float max_x = min_x + render_info.cursor_char_width;
		if (render_info.cursor_char_width == 0) {
			max_x = render_info.last_x;
		}
		float min_y = editor_state.container.maxy - ((font_rendering.max_height) * render_info.cursor_line) + font_rendering.descent;
		float max_y = editor_state.container.maxy - ((font_rendering.max_height) * (render_info.cursor_line - 1)) - (font_rendering.max_height - font_rendering.ascent);
		render_transparent_quad(min_x, min_y,
			max_x, max_y,
			&cursor_color);
	}
}

internal void render_editor_ascii_mode()
{
	// render text in the buffer
	Font_Render_Info render_info = { 0 };
	if (editor_state.render) {
		vec4 font_color = (vec4) { 0.8f, 0.8f, 0.8f, 1.0f };
		//memset(&render_info, 0, sizeof(Font_Render_Info));
		render_info.cursor_position = editor_state.cursor;
		render_info.current_line = 0;
		render_info.flags = 0 | render_info_exit_on_line_feed;

		int written = 0;
		float offset_y = 0, offset_x = 0;
		while (written < editor_state.buffer_size) {
			render_info.flags = 0 | render_info_exit_on_line_feed;
			render_info.current_line++;
			render_info.in_offset = written;
			written += render_text(editor_state.container.minx, editor_state.container.maxy - font_rendering.max_height + offset_y,
				editor_state.buffer + written, editor_state.buffer_size - written,
				editor_state.container.maxx, &font_color, &render_info);

			if (render_info.flags & render_info_exited_on_line_feed) {
				written++;
			}
			if (render_info.last_x + font_rendering.max_width >= editor_state.container.maxx ||
				render_info.flags & render_info_exited_on_line_feed) {
				offset_y -= font_rendering.max_height;
				offset_x = 0.0f;
			} else {
				offset_x = render_info.last_x + 4.0f;
			}

			// this hangs because when minimized the win_state.win_width and win_state.win_height is 0
			// and thus editor_state.container.maxx is negative, making render_text always return 0
			// and therefore never breaking out of the loop
			if (written == 0) break;
		}

		//editor_state.cursor_column = MAX(render_info.cursor_column, editor_state.cursor_column);
		editor_state.cursor_line_char_count = render_info.cursor_line_char_count;
		editor_state.cursor_prev_line_char_count = render_info.cursor_prev_line_char_count;
		// render cursor overtop
		vec4 cursor_color = (vec4) { 0.5f, 0.9f, 0.85f, 0.5f };

		float min_x = render_info.advance_x_cursor;
		float max_x = min_x + render_info.cursor_char_width;
		if (render_info.cursor_char_width == 0) {
			max_x = render_info.last_x;
		}
		float min_y = editor_state.container.maxy - ((font_rendering.max_height) * render_info.cursor_line) + font_rendering.descent;
		float max_y = editor_state.container.maxy - ((font_rendering.max_height) * (render_info.cursor_line - 1)) - (font_rendering.max_height - font_rendering.ascent + font_rendering.descent);
		render_transparent_quad(min_x, min_y,
			max_x, max_y,
			//min_x + 1.0f, editor_state.container.maxy - ((font_rendering.max_height) * (render_info.cursor_line - 1)),
			&cursor_color);
	}
	if (editor_state.debug) {
		render_debug_info(&render_info);
	}
}

void render_editor()
{
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
}

internal Editor_Mode next_mode() {
	int mode = editor_state.mode;
	mode++;
	if (mode >= EDITOR_MODE_END) mode = 0;
	return mode;
}

void handle_key_down(s32 key)
{
	s64 cursor = editor_state.cursor;

#if DEBUG
	if (key == 'R') recompile_font_shader();
	if (key == 'F') debug_toggle_font_boxes();

	if (key == 'G') {
		release_font();
		u8 font[] = "c:/windows/fonts/times.ttf";
		load_font(font, 20);
	}
	if (key == 'H') {
		release_font();
		u8 font[] = "res/LiberationMono-Regular.ttf";
		load_font(font, 20);
	}
	if (key == 'J') {
		release_font();
		u8 font[] = "c:/windows/fonts/consola.ttf";
		load_font(font, 20);
	}
	if (key == 'P') {
		editor_state.mode = next_mode();
	}

#endif

	if (key == VK_RIGHT) {
		s64 increment = 1;
		if (keyboard_state.key[17]) increment = 8;
		editor_state.cursor = MIN(editor_state.cursor + increment, editor_state.buffer_size - 1);
	}
	if (key == VK_LEFT) {
		s64 increment = 1;
		if (keyboard_state.key[17]) increment = 8;
		editor_state.cursor = MAX(editor_state.cursor - increment, 0);
	}

	if (key == VK_UP) {
		int c = editor_state.cursor_prev_line_char_count;
		if (editor_state.cursor - c > 0) {
			editor_state.cursor -= c;
		}
	}
	if (key == VK_DOWN) {
		int c = editor_state.cursor_line_char_count - editor_state.cursor_column;
		c += editor_state.cursor_column;
		if (editor_state.cursor + c < editor_state.buffer_size) {
			editor_state.cursor += c;
		}
	}

	if (editor_state.cursor != cursor && editor_state.cursor < editor_state.buffer_size) {
		//set_cursor_begin(editor_state.cursor);
	}
}

void insert_text_test(char c)
{
	if (c != 8)
	{
		insert_text(&c, 1, editor_state.cursor);
		++editor_state.cursor;
	}
	else
	{
		if (editor_state.cursor > 0)
		{
			delete_text(1, editor_state.cursor - 1);
			--editor_state.cursor;
		}
	}
}
