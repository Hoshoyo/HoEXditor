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

Editor_State* editor_state = 0;
Editor_State editor_state_data = {0};
internal Editor_State dialog_state = { 0 };

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

void bind_editor(Editor_State* es) {
	editor_state = es;
}

void init_dialog_text();

internal setup_view_buffer(s64 offset, s64 size, bool force_loading) {
	if (offset < editor_state->buffer_size && !force_loading) {
		set_cursor_begin(editor_state->main_buffer_id, offset);
	} else {
		editor_state->buffer = get_text_buffer(editor_state->main_buffer_id, size, offset);
		editor_state->buffer_valid_bytes = _tm_valid_bytes[editor_state->main_buffer_id];
		editor_state->buffer_size = _tm_text_size[editor_state->main_buffer_id];
	}
}

int max_in_a_line = 0;

void init_editor()
{
	editor_state = &editor_state_data;
	init_timer();
	char font[] = "c:/windows/fonts/consola.ttf";
	s32 font_size = 16;	// @TEMPORARY @TODO make this configurable
	init_font(font, font_size, win_state.win_width, win_state.win_height);
	init_interface();

	int id;
	load_file(&editor_state->main_buffer_id, "./res/editor.c");

	u8 word_to_search[256] = "Buddha";
	ho_search_result* result = search_word(editor_state->main_buffer_id, 0, _tm_text_size[editor_state->main_buffer_id] - 1, word_to_search, hstrlen(word_to_search));

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
	editor_state->cursor_info.cursor_offset = 0;
	editor_state->cursor_info.cursor_column = 0;
	editor_state->cursor_info.cursor_snaped_column = 0;
	editor_state->cursor_info.previous_line_count = 0;
	editor_state->cursor_info.next_line_count = 0;
	editor_state->cursor_info.this_line_count = 0;
	editor_state->cursor_info.cursor_line = 0;
	editor_state->cursor_info.block_offset = 0;

	setup_view_buffer(0, SCREEN_BUFFER_SIZE, true);

	editor_state->console_active = false;
	editor_state->render = true;
	editor_state->debug = true;
	editor_state->line_wrap = false;
	editor_state->mode = EDITOR_MODE_ASCII;

	editor_state->cursor_info.handle_seek = false;

	INIT_TEXT_CONTAINER(editor_state->console_info.container, 0.0f, win_state.win_width, 0.0f, MIN(200.0f, win_state.win_height / 2.0f), 0.0f, 0.0f, 0.0f, 0.0f);

	// @temporary initialization of container for the editor
	INIT_TEXT_CONTAINER(editor_state->container, 0.0f, 0.0f, 0.0f, 0.0f, 20.0f, 200.0f, 2.0f + font_rendering->max_height, 20.0f);
	ui_update_text_container_paddings(&editor_state->container);
	update_container(&editor_state->container);

	prepare_editor_text();
	init_dialog_text();
}

void update_container(Text_Container* container)
{
	// this has to change the state of the editor because when the window is minimized
	// it cannot render
	container->minx = container->left_padding;
	container->maxx = MAX(0.0f, win_state.win_width - container->right_padding);
	container->miny = container->bottom_padding;

	if (editor_state->console_active) container->miny = editor_state->console_info.container.maxy;

	container->maxy = MAX(0.0f, win_state.win_height - container->top_padding);
	if (win_state.win_width == 0 || win_state.win_height == 0 || container->maxx == 0 || container->maxy == 0) {
		editor_state->render = false;
	} else if(win_state.win_width > 0 && win_state.win_height > 0) {
		editor_state->render = true;
	}
}

#define CURSOR_RELATIVE_OFFSET (editor_state->cursor_info.cursor_offset - editor_state->cursor_info.block_offset)
#define SELECTION_RELATIVE_OFFSET (editor_state->cursor_info.selection_offset - editor_state->cursor_info.block_offset)

internal void render_selection(int num_lines, int num_bytes, int line_written, Font_RenderOutInfo* out_info) {
	float min_y = editor_state->container.maxy - ((font_rendering->max_height) * (float)(num_lines - 1)) + font_rendering->descent;
	float max_y = editor_state->container.maxy - ((font_rendering->max_height) * (float)(num_lines - 2)) + font_rendering->descent;
	float max_x = 0.0f;
	float min_x = 0.0f;

	// if the selection is happening with the selection cursor back
	if (editor_state->cursor_info.selection_offset < editor_state->cursor_info.cursor_offset) {
		int line_count = CURSOR_RELATIVE_OFFSET - (num_bytes - line_written);
		int selec_count = SELECTION_RELATIVE_OFFSET - (num_bytes - line_written);
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
		} else if (num_bytes - line_written >= SELECTION_RELATIVE_OFFSET &&
			num_bytes <= CURSOR_RELATIVE_OFFSET) {
			min_x = out_info->begin_width;
			max_x = out_info->exit_width;
		}
		else if (num_bytes - line_written <= CURSOR_RELATIVE_OFFSET) {
			min_x = out_info->begin_width;
			max_x = out_info->cursor_minx;
		}
	}
	// if the selection is happening with the selection cursor forward
	else if (editor_state->cursor_info.selection_offset > editor_state->cursor_info.cursor_offset) {
		int line_count = CURSOR_RELATIVE_OFFSET - (num_bytes - line_written);
		int selec_count = SELECTION_RELATIVE_OFFSET - (num_bytes - line_written);
		bool is_cursor_in_this_line = (line_count < line_written && line_count >= 0) ? true : false;
		bool is_selection_in_this_line = (selec_count < line_written && selec_count >= 0) ? true : false;
		if (is_selection_in_this_line) {
			if (is_cursor_in_this_line) {
				min_x = out_info->cursor_minx;
				//min_x = out_info->cursor_maxx;
				max_x = out_info->selection_minx;
			} else {
				min_x = out_info->begin_width;
				max_x = out_info->selection_minx;
			}
		} else if (num_bytes - line_written >= CURSOR_RELATIVE_OFFSET &&
			num_bytes <= SELECTION_RELATIVE_OFFSET) {
			min_x = out_info->begin_width;
			max_x = out_info->exit_width;
		} else if (is_cursor_in_this_line) {
			//min_x = out_info->cursor_maxx; // this is for insert mode
			min_x = out_info->cursor_minx + 1.0f;
			max_x = out_info->exit_width;
		}
	}
	vec4 selection_color = (vec4) { 0.4f, 0.5f, 0.7f, 0.4f };
	render_transparent_quad(min_x, min_y, max_x, max_y, &selection_color);
}

internal void render_editor_hex_mode()
{
	glEnable(GL_SCISSOR_TEST);
	glScissor(editor_state->container.minx, editor_state->container.miny, editor_state->container.maxx, editor_state->container.maxy);

	// render text in the buffer
	if (editor_state->render) {
		vec4 font_color = (vec4) { 0.8f, 0.8f, 0.8f, 1.0f };

		// Setup the rendering info needed to render hex
		Font_RenderInInfo in_info = { 0 };
		Font_RenderOutInfo out_info = { 0 };
		{
			in_info.cursor_offset = -1;
			in_info.exit_on_max_width = true;
			in_info.max_width = editor_state->container.maxx;
			in_info.seek_location = false;
			in_info.selection_offset = -1;
			editor_state->cursor_info.this_line_count = -1;
			editor_state->cursor_info.previous_line_count = -1;
			editor_state->cursor_info.next_line_count = -1;

			if (editor_state->cursor_info.handle_seek) {
				in_info.location_to_seek.x = editor_state->cursor_info.seek_position.x;
				in_info.location_to_seek.y = editor_state->cursor_info.seek_position.y;
				in_info.seek_location = true;
				editor_state->cursor_info.handle_seek = false;
			}
		}

		const float left_text_padding = 20.0f;	// in pixels
		const float spacing = 4.0f;				// in pixels

		float offset_y = 0, offset_x = left_text_padding;
		int written = 0, num_bytes = 0, num_lines = 1, cursor_line = 0, selection_line = 0;
		int last_line_count = 0;
		int line_count = 0;

		while (num_bytes < _tm_valid_bytes[editor_state->main_buffer_id]) {
			char hexbuffer[64];
			u64 num = *(editor_state->buffer + num_bytes);
			int num_len = u8_to_str_base16(num, false, hexbuffer);

			if (num_bytes == editor_state->cursor_info.cursor_offset) {
				in_info.cursor_offset = 0;
				cursor_line = num_lines;
			} else {
				in_info.cursor_offset = -1;
			}

			if (num_bytes <= editor_state->cursor_info.selection_offset &&
				editor_state->cursor_info.selection_offset != editor_state->cursor_info.cursor_offset) {
				in_info.selection_offset = editor_state->cursor_info.selection_offset - num_bytes;
				selection_line = num_lines;
			}
			else {
				in_info.selection_offset = -1;
			}

			written = prerender_text(editor_state->container.minx + offset_x, editor_state->container.maxy - font_rendering->max_height + offset_y,
				hexbuffer, num_len, &out_info, &in_info);

			// test seeking cursor from click
			if (out_info.seeked_index != -1) {
				editor_state->cursor_info.cursor_offset = num_bytes;
			}

			if (out_info.exited_on_limit_width) {
				// set the count of characters rendered from the cursor previous, current and next lines
				if (cursor_line == num_lines) {
					editor_state->cursor_info.this_line_count = line_count;
					editor_state->cursor_info.previous_line_count = last_line_count;
					if(in_info.cursor_offset >= 0) editor_state->cursor_info.cursor_column = in_info.cursor_offset;
				}
				if (cursor_line + 1 == num_lines) {
					editor_state->cursor_info.next_line_count = line_count;
				}

				offset_y -= font_rendering->max_height;
				offset_x = left_text_padding;
				num_lines++;
				last_line_count = line_count;
				line_count = 0;
				continue;
			} else {
				line_count += written / 2;
				queue_text(editor_state->container.minx + offset_x, editor_state->container.maxy - font_rendering->max_height + offset_y, hexbuffer, num_len);
				offset_x = (out_info.exit_width - editor_state->container.minx) + spacing;
			}
			editor_state->cursor_info.cursor_line = cursor_line;

			if (editor_state->selecting) {
				render_selection(num_lines + 1, num_bytes, written, &out_info);
			}

			num_bytes++;
			if (written == 0) break;	// if the space to render is too small for a single character than just leave
		}
		editor_state->cursor_info.last_line = num_lines - 1;
		flush_text_batch(&font_color, num_bytes * 2);

		// render cursor overtop
		vec4 cursor_color = (vec4) { 0.5f, 0.9f, 0.85f, 0.5f };
		float min_y = editor_state->container.maxy - ((font_rendering->max_height) * (float)cursor_line) + font_rendering->descent;
		float max_y = editor_state->container.maxy - ((font_rendering->max_height) * (float)(cursor_line - 1)) + font_rendering->descent;
		render_transparent_quad(out_info.cursor_minx, min_y, out_info.cursor_maxx, max_y, &cursor_color);

		// selection
		vec4 select_cursor_color = (vec4) { 0.7f, 0.9f, 0.85f, 0.5f };
		if (editor_state->selecting && editor_state->cursor_info.selection_offset != editor_state->cursor_info.cursor_offset) {
			float min_selec_y = editor_state->container.maxy - ((font_rendering->max_height) * (float)selection_line) + font_rendering->descent;
			float max_selec_y = editor_state->container.maxy - ((font_rendering->max_height) * (float)(selection_line - 1)) + font_rendering->descent;
			render_transparent_quad(out_info.selection_minx, min_selec_y, out_info.selection_maxx, max_selec_y, &select_cursor_color);
		}

		glDisable(GL_SCISSOR_TEST);
	}
}

internal void render_editor_ascii_mode()
{
	glEnable(GL_SCISSOR_TEST);
	glScissor(editor_state->container.minx, editor_state->container.miny, editor_state->container.maxx, editor_state->container.maxy);

	if (editor_state->render) {
		vec4 font_color = (vec4) { 0.8f, 0.8f, 0.8f, 1.0f };

		Font_RenderInInfo in_info = { 0 };
		Font_RenderOutInfo out_info = { 0 };

		float min_height = editor_state->container.miny;

		{
			in_info.cursor_offset = -1;
			in_info.exit_on_max_width = editor_state->line_wrap;
			in_info.max_width = editor_state->container.maxx;
			in_info.exit_on_line_feed = true;
			in_info.seek_location = false;
			in_info.selection_offset = -1;
			editor_state->cursor_info.this_line_count = -1;
			editor_state->cursor_info.previous_line_count = -1;
			editor_state->cursor_info.next_line_count = -1;

			if (editor_state->cursor_info.handle_seek) {
				in_info.location_to_seek.x = editor_state->cursor_info.seek_position.x;
				in_info.location_to_seek.y = editor_state->cursor_info.seek_position.y;
				in_info.seek_location = true;
				editor_state->cursor_info.handle_seek = false;
			}
		}

		int written = 0, num_bytes = 0, cursor_line = 0, num_lines = 1, selection_line = 0;
		float offset_y = 0, offset_x = 0;
		int last_line_count = 0;
		int cursor_location = -1;
		max_in_a_line = 0;
		bool exited_on_limit_height = false;

		while (num_bytes <= editor_state->buffer_valid_bytes) {
			if (num_bytes <= CURSOR_RELATIVE_OFFSET) {
				in_info.cursor_offset = CURSOR_RELATIVE_OFFSET - num_bytes;
				cursor_line = num_lines;
			} else {
				in_info.cursor_offset = -1;
			}

			if (num_bytes <= SELECTION_RELATIVE_OFFSET &&
				editor_state->cursor_info.selection_offset != editor_state->cursor_info.cursor_offset) {
				in_info.selection_offset = SELECTION_RELATIVE_OFFSET - num_bytes;
				selection_line = num_lines;
			} else {
				in_info.selection_offset = -1;
			}

			s64 num_to_write = editor_state->buffer_size - num_bytes - (editor_state->buffer_size - editor_state->buffer_valid_bytes);
			written = prerender_text(editor_state->container.minx, editor_state->container.maxy - font_rendering->max_height + offset_y,
				editor_state->buffer + num_bytes, num_to_write, &out_info, &in_info);

			if (out_info.seeked_index != -1) {
				// test seeking cursor from click
				editor_state->cursor_info.cursor_offset = editor_state->cursor_info.block_offset + num_bytes + out_info.seeked_index;
			}
			if (in_info.cursor_offset == num_to_write && out_info.exited_on_line_feed) {
				// needed for when the cursor is in a new line by itself
				cursor_line++;
			}
			// optional
			if (in_info.cursor_offset >= written) {
				cursor_location = 1;
			} else {
				cursor_location = 0;
			}

			queue_text(editor_state->container.minx, editor_state->container.maxy - font_rendering->max_height + offset_y, editor_state->buffer + num_bytes, written);

			// set the count of characters rendered from the cursor previous, current and next lines
			if (cursor_line == num_lines) {
				editor_state->cursor_info.this_line_count = written;
				editor_state->cursor_info.previous_line_count = last_line_count;
				editor_state->cursor_info.cursor_column = written - (num_bytes + written - CURSOR_RELATIVE_OFFSET);
			}
			if (cursor_line + 1 == num_lines) {
				editor_state->cursor_info.next_line_count = MAX(1, written);
			}
			
			if (num_lines == 1) {
				editor_state->first_line_count = written;
			}

			offset_y -= font_rendering->max_height;
			offset_x = 0.0f;
			num_lines++;
			last_line_count = written;

			if (written > max_in_a_line) max_in_a_line = written;
			num_bytes += written;

			if (editor_state->selecting) {
				render_selection(num_lines, num_bytes, written, &out_info);
			}

			if (editor_state->container.maxy - font_rendering->max_height + offset_y < min_height) {
				exited_on_limit_height = true;
				break;
			}

			if (written == 0) break;	// if the space to render is too small for a single character than just leave
		}

		if (cursor_line == 0) cursor_line = 1;
		editor_state->last_line_count = last_line_count;
		editor_state->cursor_info.last_line = num_lines - 1;
		editor_state->cursor_info.cursor_line = cursor_line;
		flush_text_batch(&font_color, num_bytes);

		// render cursor overtop
		//if (cursor_location != 0) return;
		vec4 cursor_color = (vec4) { 0.6f, 0.7f, 0.85f, 0.8f };
		float min_y = editor_state->container.maxy - ((font_rendering->max_height) * (float)cursor_line) + font_rendering->descent;
		float max_y = editor_state->container.maxy - ((font_rendering->max_height) * (float)(cursor_line - 1)) + font_rendering->descent;
		float min_x = MAX(out_info.cursor_minx, editor_state->container.minx);
		float max_x = min_x + 1.0f;
		render_transparent_quad(min_x, min_y, max_x, max_y, &cursor_color);

		// selection
		vec4 select_cursor_color = (vec4) { 0.7f, 0.9f, 0.85f, 0.5f };
		if (editor_state->selecting && editor_state->cursor_info.selection_offset != editor_state->cursor_info.cursor_offset) {
			float min_selec_y = editor_state->container.maxy - ((font_rendering->max_height) * (float)selection_line) + font_rendering->descent;
			float max_selec_y = editor_state->container.maxy - ((font_rendering->max_height) * (float)(selection_line - 1)) + font_rendering->descent;
			render_transparent_quad(out_info.selection_minx, min_selec_y, out_info.selection_maxx, max_selec_y, &select_cursor_color);
		}
	}

	glDisable(GL_SCISSOR_TEST);
}

void init_dialog_text() {
	// init cursor state
	dialog_state.cursor_info.cursor_offset = 0;
	dialog_state.cursor_info.cursor_column = 0;
	dialog_state.cursor_info.cursor_snaped_column = 0;
	dialog_state.cursor_info.previous_line_count = 0;
	dialog_state.cursor_info.next_line_count = 0;
	dialog_state.cursor_info.this_line_count = 0;
	dialog_state.cursor_info.cursor_line = 0;
	dialog_state.buffer = halloc(1024);	// @temporary
	dialog_state.buffer_size = 1024;

	dialog_state.console_active = false;
	dialog_state.render = true;
	dialog_state.debug = true;
	dialog_state.mode = EDITOR_MODE_ASCII;

	INIT_TEXT_CONTAINER(dialog_state.container, 0.0f, win_state.win_width, 0.0f, font_rendering->max_height, 0.0f, 0.0f, 0.0f, 0.0f);
}

void render_dialog_text(s64 cursor_position, u8* text, s64 text_size) {
	vec4 font_color = (vec4) { 0.8f, 0.8f, 0.8f, 1.0f };

	dialog_state.cursor_info.cursor_offset = cursor_position;

	Font_RenderInInfo in_info = { 0 };
	Font_RenderOutInfo out_info = { 0 };

	{
		in_info.cursor_offset = -1;
		in_info.exit_on_max_width = true;
		in_info.max_width = win_state.win_width;
		in_info.exit_on_line_feed = true;
		in_info.seek_location = false;
		in_info.selection_offset = -1;
		dialog_state.cursor_info.this_line_count = -1;
		dialog_state.cursor_info.previous_line_count = -1;
		dialog_state.cursor_info.next_line_count = -1;

		if (dialog_state.cursor_info.handle_seek) {
			in_info.location_to_seek.x = dialog_state.cursor_info.seek_position.x;
			in_info.location_to_seek.y = dialog_state.cursor_info.seek_position.y;
			in_info.seek_location = true;
			dialog_state.cursor_info.handle_seek = false;
		}
	}

	int written = 0, num_bytes = 0, cursor_line = 0, num_lines = 1, selection_line = 0;
	float offset_y = 0, offset_x = 0;
	int last_line_count = 0;

	while (num_bytes < text_size) {

		if (num_bytes <= dialog_state.cursor_info.cursor_offset) {
			in_info.cursor_offset = dialog_state.cursor_info.cursor_offset - num_bytes;
			cursor_line = num_lines;
		}
		else {
			in_info.cursor_offset = -1;
		}

		if (num_bytes <= dialog_state.cursor_info.selection_offset &&
			dialog_state.cursor_info.selection_offset != dialog_state.cursor_info.cursor_offset) {
			in_info.selection_offset = dialog_state.cursor_info.selection_offset - num_bytes;
			selection_line = num_lines;
		}
		else {
			in_info.selection_offset = -1;
		}

		s64 num_to_write = text_size - num_bytes;
		written = prerender_text(dialog_state.container.minx, dialog_state.container.maxy - font_rendering->max_height + offset_y,
			text + num_bytes, num_to_write, &out_info, &in_info);

		// test seeking cursor from click
		if (out_info.seeked_index != -1) {
			dialog_state.cursor_info.cursor_offset = num_bytes + out_info.seeked_index;
		}

		written = render_text(dialog_state.container.minx, dialog_state.container.maxy - font_rendering->max_height + offset_y,
			text + num_bytes, written, &font_color);

		// set the count of characters rendered from the cursor previous, current and next lines
		if (cursor_line == num_lines) {
			dialog_state.cursor_info.this_line_count = written;
			dialog_state.cursor_info.previous_line_count = last_line_count;
			dialog_state.cursor_info.cursor_column = written - (num_bytes + written - dialog_state.cursor_info.cursor_offset);
		}
		if (cursor_line + 1 == num_lines) {
			dialog_state.cursor_info.next_line_count = written;
		}

		offset_y -= font_rendering->max_height;
		offset_x = 0.0f;
		num_lines++;
		last_line_count = written;

		num_bytes += written;

		if (dialog_state.selecting) {
			render_selection(num_lines, num_bytes, written, &out_info);
		}

		if (written == 0) break;	// if the space to render is too small for a single character than just leave
	}
	editor_state->cursor_info.last_line = num_lines - 1;
	dialog_state.cursor_info.cursor_line = cursor_line;
	// render cursor overtop
	vec4 cursor_color = (vec4) { 0.6f, 0.7f, 0.85f, 0.8f };
	float min_y = dialog_state.container.maxy - ((font_rendering->max_height) * (float)cursor_line) + font_rendering->descent;
	float max_y = dialog_state.container.maxy - ((font_rendering->max_height) * (float)(cursor_line - 1)) + font_rendering->descent;
	float min_x = out_info.cursor_minx;
	float max_x = min_x + 1.0f;
	render_transparent_quad(min_x, min_y, max_x, max_y, &cursor_color);

	// selection
	vec4 select_cursor_color = (vec4) { 0.7f, 0.9f, 0.85f, 0.5f };
	if (dialog_state.selecting && dialog_state.cursor_info.selection_offset != dialog_state.cursor_info.cursor_offset) {
		float min_selec_y = dialog_state.container.maxy - ((font_rendering->max_height) * (float)selection_line) + font_rendering->descent;
		float max_selec_y = dialog_state.container.maxy - ((font_rendering->max_height) * (float)(selection_line - 1)) + font_rendering->descent;
		render_transparent_quad(out_info.selection_minx, min_selec_y, out_info.selection_maxx, max_selec_y, &select_cursor_color);
	}
}

void render_console()
{
	vec4 console_bg_color = (vec4) { 0.0f, 0.0f, 0.1f, 0.8f };
	render_transparent_quad(
		editor_state->console_info.container.minx,
		editor_state->console_info.container.miny,
		editor_state->console_info.container.maxx,
		editor_state->console_info.container.maxy,
		&console_bg_color);

	memcpy(&dialog_state.container, &editor_state->console_info.container, sizeof(Text_Container));

	bind_editor(&dialog_state);
	s64 buffer_offset = 0;

	copy_string(editor_state->buffer + buffer_offset, "Cursor offset: ", sizeof "Cursor offset: " - 1);
	buffer_offset = sizeof "Cursor offset: " - 1;
	int n = s64_to_str_base10(editor_state_data.cursor_info.cursor_offset, editor_state->buffer + buffer_offset);
	buffer_offset += n;

	copy_string(editor_state->buffer + buffer_offset, "\nNext line count: ", sizeof("\nNext line count: ") - 1);
	buffer_offset += sizeof("Next line count: ") - 1;
	n = s64_to_str_base10(editor_state_data.cursor_info.next_line_count, editor_state->buffer + buffer_offset);
	buffer_offset += n;

	copy_string(editor_state->buffer + buffer_offset, "\nPrev line count: ", sizeof("\nPrev line count: ") - 1);
	buffer_offset += sizeof("\nPrev line count: ") - 1;
	n = s64_to_str_base10(editor_state_data.cursor_info.previous_line_count, editor_state->buffer + buffer_offset);
	buffer_offset += n;

	copy_string(editor_state->buffer + buffer_offset, "\nSnap cursor column: ", sizeof("\nSnap cursor column: ") - 1);
	buffer_offset += sizeof("\nSnap cursor column: ") - 1;
	n = s64_to_str_base10(editor_state_data.cursor_info.cursor_snaped_column, editor_state->buffer + buffer_offset);
	buffer_offset += n;

	copy_string(editor_state->buffer + buffer_offset, "\nCursor column: ", sizeof("\nCursor column: ") - 1);
	buffer_offset += sizeof("\nCursor column: ") - 1;
	n = s64_to_str_base10(editor_state_data.cursor_info.cursor_column, editor_state->buffer + buffer_offset);
	buffer_offset += n;

	editor_state->buffer_valid_bytes = buffer_offset;
	render_editor_ascii_mode();

}

void editor_start_selection() {
	if (!editor_state->selecting) {
		editor_state->cursor_info.selection_offset = editor_state->cursor_info.cursor_offset;
	}
	editor_state->selecting = true;
}

void editor_end_selection() {
	if(editor_state->selecting) editor_state->selecting = true;
}

void editor_reset_selection(){
	editor_state->selecting = false;
}

void render_editor()
{
	Editor_State* prev_es = editor_state;

	bind_editor(&editor_state_data);
	editor_state->buffer_size = MIN(SCREEN_BUFFER_SIZE, _tm_text_size[editor_state->main_buffer_id]);
	editor_state->buffer_valid_bytes = _tm_valid_bytes[editor_state->main_buffer_id];
	render_interface();
	update_container(&editor_state->container);

	switch (editor_state->mode) {
	case EDITOR_MODE_ASCII: {
		render_editor_ascii_mode();
	}break;
	case EDITOR_MODE_HEX: {
		render_editor_hex_mode();
	}break;
	}

	editor_state->cursor_info.handle_seek = false;

	if (editor_state->console_active) {
		render_console();
	}

	bind_editor(prev_es);
}

internal Editor_Mode next_mode() {
	int mode = editor_state->mode;
	mode++;
	if (mode >= EDITOR_MODE_END) mode = 0;
	return mode;
}

void update_buffer() {
	Editor_State* prev = editor_state;
	bind_editor(&editor_state_data);

	setup_view_buffer(0, SCREEN_BUFFER_SIZE, true);

	editor_state->cursor_info.cursor_offset = 0;
	editor_state->cursor_info.block_offset = 0;

	bind_editor(prev);
}

internal void handle_key_down_hex(s32 key, bool selection_reset) {
	s64 cursor = editor_state->cursor_info.cursor_offset;
}

internal void scroll_down_ascii() {
	setup_view_buffer(editor_state->cursor_info.block_offset + editor_state->first_line_count, SCREEN_BUFFER_SIZE, false);
	editor_state->cursor_info.block_offset += editor_state->first_line_count;
}
internal void scroll_up_ascii(s64 new_line_count) {
	setup_view_buffer(editor_state->cursor_info.block_offset - new_line_count, SCREEN_BUFFER_SIZE, false);
	editor_state->cursor_info.block_offset -= new_line_count;
}

internal void handle_key_down_ascii(s32 key, bool selection_reset) {
	s64 cursor = editor_state->cursor_info.cursor_offset;

	cursor_info cinfo;
	if (key == VK_UP || key == VK_DOWN || key == VK_LEFT || key == VK_RIGHT || key == VK_HOME || key == VK_END)
	{
		// selection_stuff
		if (keyboard_state.key[VK_SHIFT]) editor_start_selection();
		else if (selection_reset) return;

#if 0	// example @temporary
		cinfo = get_cursor_info(editor_state->main_buffer_id, editor_state->cursor_info.cursor_offset + 1);
		print("CURSOR INFO: \n");
		print("LINE: %d\nPREVIOUS LINE BREAK: %d\nNEXT LINE BREAK: %d\n",
			cinfo.line_number.lf,
			editor_state->cursor_info.cursor_offset - cinfo.previous_line_break.lf,
			cinfo.next_line_break.lf - editor_state->cursor_info.cursor_offset);
#endif
	}

	if (key == VK_RIGHT) {
		s64 increment = 1;
		if (keyboard_state.key[17]) increment = 8;

		if (editor_state->selecting && editor_state->cursor_info.cursor_offset == editor_state->cursor_info.selection_offset) {
			editor_state->cursor_info.selection_offset = editor_state->cursor_info.cursor_offset;
		}
		editor_state->cursor_info.cursor_snaped_column = editor_state->cursor_info.cursor_column + increment;
		if (editor_state->cursor_info.cursor_snaped_column >= editor_state->cursor_info.this_line_count) {
			editor_state->cursor_info.cursor_snaped_column = editor_state->cursor_info.cursor_snaped_column - editor_state->cursor_info.this_line_count;
		}
		editor_state->cursor_info.cursor_offset = MIN(editor_state->cursor_info.cursor_offset + increment, editor_state->buffer_valid_bytes + editor_state->cursor_info.block_offset);
		if (editor_state->cursor_info.cursor_offset == editor_state->buffer_valid_bytes + editor_state->cursor_info.block_offset) return;	// dont pass the size of buffer
		if (editor_state->cursor_info.cursor_line == editor_state->cursor_info.last_line) {
			if (editor_state->cursor_info.this_line_count - 1 == editor_state->cursor_info.cursor_column) {
				// this should be the last character on screen so scroll down
				scroll_down_ascii();
			}
		}
	}
	if (key == VK_LEFT) {
		s64 increment = 1;
		if (keyboard_state.key[17]) increment = 8;

		if (editor_state->selecting && editor_state->cursor_info.cursor_offset == editor_state->cursor_info.selection_offset) {
			editor_state->cursor_info.selection_offset = editor_state->cursor_info.cursor_offset;
		}
		editor_state->cursor_info.cursor_snaped_column = editor_state->cursor_info.cursor_column - increment;
		if (editor_state->cursor_info.cursor_snaped_column < 0) {
			s64 new_snap = editor_state->cursor_info.previous_line_count + editor_state->cursor_info.cursor_snaped_column;
			editor_state->cursor_info.cursor_snaped_column = new_snap;
		}

		if(CURSOR_RELATIVE_OFFSET - increment < 0){
			// go back one line on the view
			u64 first_char_pos = editor_state->cursor_info.cursor_offset - editor_state->cursor_info.cursor_column;
			if (first_char_pos == 0) return;	// if this is the first character, no need to go left
			cinfo = get_cursor_info(editor_state->main_buffer_id, first_char_pos - 1);
			s64 amount_last_line = (first_char_pos - 1) - cinfo.previous_line_break.lf;

			s64 back_amt = MAX(0, editor_state->cursor_info.cursor_offset - increment);
			editor_state->cursor_info.cursor_offset = back_amt;

			scroll_up_ascii(amount_last_line);
		} else {
			// go back normally because the line is in view
			editor_state->cursor_info.cursor_offset = MAX(editor_state->cursor_info.cursor_offset - increment, 0);
		}
	}

	if (key == VK_UP) {
		int snap = editor_state->cursor_info.cursor_snaped_column;
		s64 back_amt = editor_state->cursor_info.cursor_offset - editor_state->cursor_info.cursor_column - 1;

		if (back_amt < 0) return;	// this is the first line in the text, no need to go up

		cinfo = get_cursor_info(editor_state->main_buffer_id, editor_state->cursor_info.cursor_offset - editor_state->cursor_info.cursor_column - 1);
		s64 previous_line_count = (editor_state->cursor_info.cursor_offset - editor_state->cursor_info.cursor_column - 1) - cinfo.previous_line_break.lf;

		int c = previous_line_count - MAX(snap, editor_state->cursor_info.cursor_column - 1);
		if (c <= 0) c = 1;
		c += editor_state->cursor_info.cursor_column;

		// if the cursor is on the first line
		if (CURSOR_RELATIVE_OFFSET - editor_state->cursor_info.cursor_column - 1 < 0) {
			assert(editor_state->cursor_info.cursor_line == 1);
			scroll_up_ascii(previous_line_count);
		}

		editor_state->cursor_info.cursor_offset -= c;
	}
	if (key == VK_DOWN) {
		if (editor_state->line_wrap) {
			// todo
		} else {
			s64 count_from_cursor_to_next_lf = get_cursor_info(editor_state->main_buffer_id, editor_state->cursor_info.cursor_offset).next_line_break.lf;
			if (count_from_cursor_to_next_lf == -1) {
				// if we are at the last line in the text
				return;
			}
			count_from_cursor_to_next_lf -= editor_state->cursor_info.cursor_offset;

			s64 count_of_next_line = get_cursor_info(editor_state->main_buffer_id, editor_state->cursor_info.cursor_offset + count_from_cursor_to_next_lf + 1).next_line_break.lf;
			if (count_of_next_line == -1) {
				// if we are at the penultima line we won't have a \n at the end of the text
				count_of_next_line = _tm_text_size[editor_state->main_buffer_id] - (editor_state->cursor_info.cursor_offset + count_from_cursor_to_next_lf + 1);

			} else {
				// otherwise proceed normally
				count_of_next_line -= editor_state->cursor_info.cursor_offset + count_from_cursor_to_next_lf + 1;
			}

			s64 cursor_column = editor_state->cursor_info.cursor_column;
			s64 snap = editor_state->cursor_info.cursor_snaped_column;
			
			s64 count_to_skip = MIN(MAX(cursor_column, snap) + count_from_cursor_to_next_lf + 1, count_from_cursor_to_next_lf + 1 + count_of_next_line);

			if (CURSOR_RELATIVE_OFFSET + count_to_skip <= editor_state->buffer_size && editor_state->cursor_info.next_line_count > 0) {
				// case in which we are inside the area of rendering
				editor_state->cursor_info.cursor_offset += count_to_skip;
			} else {
				// the next line is outside the view of the window
				editor_state->cursor_info.cursor_offset += count_to_skip;
				scroll_down_ascii();
			}
		}		
	}

	if (key == VK_HOME) {
		editor_state->cursor_info.cursor_offset -= editor_state->cursor_info.cursor_column;
		editor_state->cursor_info.cursor_snaped_column = 0;
	}
	if (key == VK_END) {
		s64 value = editor_state->cursor_info.this_line_count - editor_state->cursor_info.cursor_column;
		s64 is_final = 0;
		if (editor_state->cursor_info.cursor_offset + value < editor_state->buffer_size) {
			value--;
			is_final++;
		}
		editor_state->cursor_info.cursor_offset += value;
		editor_state->cursor_info.cursor_snaped_column = editor_state->cursor_info.this_line_count + is_final;
	}
}

void handle_key_down(s32 key)
{
	bool selection_reset = false;
	if ((key == VK_LEFT || key == VK_RIGHT || key == VK_UP || key == VK_DOWN || key == VK_END || key == VK_HOME)
		&& !keyboard_state.key[VK_SHIFT] && key != BACKSPACE_KEY && !keyboard_state.key[BACKSPACE_KEY]) {
		if (!keyboard_state.key[CTRL_KEY]) {
			if(editor_state->selecting) selection_reset = true;
			editor_state->selecting = false;
		}
	}

	if (key == 'R' && keyboard_state.key[17]) { recompile_font_shader(); return; }
	if (key == 'P' && keyboard_state.key[17]) {
		editor_state->mode = next_mode();
		editor_state->cursor_info.cursor_snaped_column = 0;
	}

	if (key == VK_F1) {
		editor_state_data.console_active = !editor_state_data.console_active;
		//if (editor_state == &dialog_state) bind_editor(&editor_state_data);
		//else bind_editor(&dialog_state);
	}

	handle_key_down_ascii(key, selection_reset);
	//handle_key_down_hex(key, selection_reset);
}

void handle_lmouse_down(int x, int y)
{
	float xf = (float)x;
	float yf = (float)y;

	yf = win_state.win_height - yf;

	editor_state->cursor_info.handle_seek = true;
	editor_state->cursor_info.seek_position.x = xf;
	editor_state->cursor_info.seek_position.y = yf;
}
