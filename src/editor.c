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
	s32 font_size = 20;	// @TEMPORARY @TODO make this configurable
	init_font(font, font_size, win_state.win_width, win_state.win_height);

	init_text_api("./res/dummy.txt");	// @temporary, init this in the proper way

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

	INIT_TEXT_CONTAINER(editor_state.console_info.container, 0.0f, win_state.win_width, 0.0f, MIN(200.0f, win_state.win_height / 2.0f), 0.0f, 0.0f, 0.0f, 0.0f);

	// @temporary initialization of container for the editor
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
	wrtn = s64_to_str_base10(editor_state.cursor_info.cursor_offset, buffer);
	wrtn = render_text(render_info.last_x, -font_rendering.descent, buffer, wrtn, win_state.win_width, &font_color, 0);
}


internal void render_editor_hex_mode()
{

	vec4 bg_color = (vec4) { 0.05f, 0.05f, 0.05f, 1.0f };
	render_transparent_quad(editor_state.container.minx, editor_state.container.miny,
		editor_state.container.maxx, editor_state.container.maxy,
		&bg_color);

	glEnable(GL_SCISSOR_TEST);
	glScissor(editor_state.container.minx, editor_state.container.miny, editor_state.container.maxx, editor_state.container.maxy);

	// render text in the buffer
	if (editor_state.render) {
		vec4 font_color = (vec4) { 0.8f, 0.8f, 0.8f, 1.0f };
		Font_Render_Info render_info = { 0 };
		render_info.cursor_position = editor_state.cursor_info.cursor_offset;
		render_info.current_line = 1;

		int written = 0, num_bytes = 0, num_lines = 1, cursor_line = 0;
		float offset_y = 0, offset_x = 0;

		// Setup the rendering info needed to render hex
		Font_RenderInInfo in_info = { 0 };
		Font_RenderOutInfo out_info = { 0 };
		in_info.cursor_offset = -1;
		in_info.exit_on_max_width = true;
		in_info.max_width = editor_state.container.maxx;

		while (num_bytes < editor_state.buffer_size) {
			//render_info.in_offset = written;
			char hexbuffer[64];
			u64 num = *(editor_state.buffer + num_bytes);
			int num_len = u8_to_str_base16(num, false, hexbuffer);

			if (num_bytes == editor_state.cursor_info.cursor_offset) {
				in_info.cursor_offset = 0;
				cursor_line = num_lines;
			}
			else {
				in_info.cursor_offset = -1;
			}

			written = prerender_text(editor_state.container.minx + offset_x, editor_state.container.maxy - font_rendering.max_height + offset_y,
				hexbuffer, num_len, &out_info, &in_info);
			if (out_info.exited_on_limit_width) {
				offset_y -= font_rendering.max_height;
				offset_x = 0.0f;
				num_lines++;
				continue;
			} else {
				render_text2(editor_state.container.minx + offset_x, editor_state.container.maxy - font_rendering.max_height + offset_y,
					hexbuffer, num_len, &font_color);
				float spacing = 4.0f;
				offset_x = out_info.exit_width + spacing;
			}

			num_bytes++;
			assert(written != 0); // prevent infinite loop
		}
		// render cursor overtop
		vec4 cursor_color = (vec4) { 0.5f, 0.9f, 0.85f, 0.5f };
		float min_y = editor_state.container.maxy - ((font_rendering.max_height) * (float)cursor_line) + font_rendering.descent;
		float max_y = editor_state.container.maxy - ((font_rendering.max_height) * (float)(cursor_line - 1)) + font_rendering.descent;
		render_transparent_quad(out_info.cursor_minx, min_y, out_info.cursor_maxx, max_y, &cursor_color);

		glDisable(GL_SCISSOR_TEST);

		// debug information
		render_debug_info(&render_info);
	}
}

internal void render_editor_ascii_mode()
{
	glEnable(GL_SCISSOR_TEST);
	glScissor(editor_state.container.minx, editor_state.container.miny, editor_state.container.maxx, editor_state.container.maxy);

	// render text in the buffer
	Font_Render_Info render_info = { 0 };
	if (editor_state.render) {
		vec4 font_color = (vec4) { 0.8f, 0.8f, 0.8f, 1.0f };
		//memset(&render_info, 0, sizeof(Font_Render_Info));
		render_info.cursor_position = editor_state.cursor_info.cursor_offset;
		render_info.current_line = 0;
		render_info.flags = 0 | render_info_exit_on_line_feed;

		int written = 0;
		float offset_y = 0, offset_x = 0;
		while (written < _tm_valid_bytes) {
			render_info.flags = 0 | render_info_exit_on_line_feed;
			render_info.current_line++;
			render_info.in_offset = written;
			s64 num_to_write = editor_state.buffer_size - written - (editor_state.buffer_size - _tm_valid_bytes);
			written += render_text(editor_state.container.minx, editor_state.container.maxy - font_rendering.max_height + offset_y,
				editor_state.buffer + written, num_to_write,
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
		editor_state.cursor_info.this_line_count = render_info.cursor_line_char_count;
		editor_state.cursor_info.previous_line_count = render_info.cursor_prev_line_char_count;
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
	glDisable(GL_SCISSOR_TEST);
	if (editor_state.debug) {
		render_debug_info(&render_info);
	}
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
	s64 cursor = editor_state.cursor_info.cursor_offset;

	if (key == 'R' && keyboard_state.key[17]) { recompile_font_shader(); return; }
	if (key == 'F' && keyboard_state.key[17]) { debug_toggle_font_boxes(); return; }
	if (key == 'P' && keyboard_state.key[17]) {	editor_state.mode = next_mode(); }
	if (key == VK_F1) {
		editor_state.console_active = !editor_state.console_active;
	}
#if DEBUG
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
#endif

	if (key == VK_RIGHT) {
		s64 increment = 1;
		if (keyboard_state.key[17]) increment = 8;
		editor_state.cursor_info.cursor_offset = MIN(editor_state.cursor_info.cursor_offset + increment, editor_state.buffer_size - 1);
	}
	if (key == VK_LEFT) {
		s64 increment = 1;
		if (keyboard_state.key[17]) increment = 8;
		editor_state.cursor_info.cursor_offset = MAX(editor_state.cursor_info.cursor_offset - increment, 0);
	}

	if (key == VK_UP) {
		int c = editor_state.cursor_info.previous_line_count;
		if (editor_state.cursor_info.cursor_offset - c > 0) {
			editor_state.cursor_info.cursor_offset -= c;
		}
	}
	if (key == VK_DOWN) {
		int c = editor_state.cursor_info.this_line_count - editor_state.cursor_info.cursor_column;
		c += editor_state.cursor_info.cursor_column;
		if (editor_state.cursor_info.cursor_offset + c < editor_state.buffer_size) {
			editor_state.cursor_info.cursor_offset += c;
		}
	}

	if (editor_state.cursor_info.cursor_offset != cursor && editor_state.cursor_info.cursor_offset < editor_state.buffer_size) {
		//set_cursor_begin(editor_state.cursor);
	}
}

void editor_insert_text(char c)
{
	if (c != 8)
	{
		insert_text(&c, 1, editor_state.cursor_info.cursor_offset);

		ho_aiv_undo_redo* aiv = halloc(sizeof(ho_aiv_undo_redo));
		aiv->text = halloc(sizeof(u8));
		*(aiv->text) = c;
		aiv->text_size = 1;
		aiv->cursor_position = editor_state.cursor_info.cursor_offset;

		ho_action_item action_item;
		action_item.type = HO_INSERT_TEXT;
		action_item.value = aiv;
		push_stack_item(HO_UNDO_STACK, action_item);
		empty_stack(HO_REDO_STACK);

		editor_state.cursor_info.cursor_offset += 1;
	}
	else
	{
		if (editor_state.cursor_info.cursor_offset > 0)
		{
			u8* deleted_text = halloc(sizeof(u8));

			delete_text(deleted_text, 1, editor_state.cursor_info.cursor_offset - 1);

			ho_aiv_undo_redo* aiv = halloc(sizeof(ho_aiv_undo_redo));
			aiv->text = deleted_text;
			aiv->text_size = 1;
			aiv->cursor_position = editor_state.cursor_info.cursor_offset - 1;

			ho_action_item action_item;
			action_item.type = HO_DELETE_TEXT;
			action_item.value = aiv;
			push_stack_item(HO_UNDO_STACK, action_item);
			empty_stack(HO_REDO_STACK);

			editor_state.cursor_info.cursor_offset -= 1;
		}
	}

	check_text();
	check_arenas();
}
