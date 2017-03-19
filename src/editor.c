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

Editor_State editor_state_data = {null};

extern Window_State win_state;

#define CURSOR_RELATIVE_OFFSET (es->cursor_info.cursor_offset - es->cursor_info.block_offset)
#define SELECTION_RELATIVE_OFFSET (es->cursor_info.selection_offset - es->cursor_info.block_offset)

#define INIT_TEXT_CONTAINER(Cont, MINX, MAXX, MINY, MAXY, LP, RP, TP, BP) \
Cont.minx = MINX;	\
Cont.maxx = MAXX;	\
Cont.miny = MINY;	\
Cont.maxy = MAXY;	\
Cont.left_padding = LP;	\
Cont.right_padding = RP; \
Cont.top_padding = TP;	\
Cont.bottom_padding = BP	\

internal void update_focused_editor_state() {
	//focused_editor_state->cursor_info.
}

internal void setup_view_buffer(Editor_State* es, s64 offset, s64 size, bool force_loading) {
	if (offset < es->buffer_size && !force_loading) {
		set_cursor_begin(es->main_buffer_id, offset);
	} else {
		es->buffer = get_text_buffer(es->main_buffer_id, size, offset);
		es->buffer_valid_bytes = _tm_valid_bytes[es->main_buffer_id];
		es->buffer_size = _tm_text_size[es->main_buffer_id];
	}
}

Editor_State* init_text_editor()
{
	char font[] = "c:/windows/fonts/consola.ttf";
	s32 font_size = 16;	// @TEMPORARY @TODO make this configurable

	init_timer();
	init_font(font, font_size, win_state.win_width, win_state.win_height);

	load_file(&editor_state_data.main_buffer_id, "./res/editor.c");

	// init editor_state_data
	editor_state_data.cursor_info.cursor_offset = 0;
	editor_state_data.cursor_info.cursor_column = 0;
	editor_state_data.cursor_info.cursor_snaped_column = 0;
	editor_state_data.cursor_info.previous_line_count = 0;
	editor_state_data.cursor_info.next_line_count = 0;
	editor_state_data.cursor_info.this_line_count = 0;
	editor_state_data.cursor_info.cursor_line = 0;
	editor_state_data.cursor_info.block_offset = 0;
	editor_state_data.font_color = FONT_COLOR;
	editor_state_data.cursor_color = CURSOR_COLOR;

	setup_view_buffer(&editor_state_data, 0, SCREEN_BUFFER_SIZE, true);
	editor_state_data.render = true;
	editor_state_data.debug = true;
	editor_state_data.line_wrap = false;
	editor_state_data.mode = EDITOR_MODE_ASCII;
	editor_state_data.is_block_text = true;
	editor_state_data.render_line_numbers = true;

	editor_state_data.cursor_info.handle_seek = false;

	// @temporary initialization of container for the editor
	INIT_TEXT_CONTAINER(editor_state_data.container, 0.0f, 0.0f, 0.0f, 0.0f, 20.0f, 200.0f, 2.0f + font_rendering->max_height, 20.0f);
	//ui_update_text_container_paddings(&editor_state_data.container);
	//update_container(&editor_state_data);

	prepare_editor_text();

	return &editor_state_data;
}

void update_container(Editor_State* es)
{
	// this has to change the state of the editor because when the window is minimized
	// it cannot render
	es->container.minx = es->container.left_padding;
	es->container.maxx = MAX(0.0f, win_state.win_width - es->container.right_padding);
	es->container.miny = es->container.bottom_padding;

	es->container.maxy = MAX(0.0f, win_state.win_height - es->container.top_padding);
	if (win_state.win_width == 0 || win_state.win_height == 0 || es->container.maxx == 0 || es->container.maxy == 0) {
		es->render = false;
	} else if(win_state.win_width > 0 && win_state.win_height > 0) {
		es->render = true;
	}
}

internal void render_selection(Editor_State* es, int num_lines, int num_bytes, int line_written, Font_RenderOutInfo* out_info) {
	float min_y = es->container.maxy - ((font_rendering->max_height) * (float)(num_lines - 1)) + font_rendering->descent;
	float max_y = es->container.maxy - ((font_rendering->max_height) * (float)(num_lines - 2)) + font_rendering->descent;
	float max_x = 0.0f;
	float min_x = 0.0f;

	// if the selection is happening with the selection cursor back
	if (es->cursor_info.selection_offset < es->cursor_info.cursor_offset) {
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
	else if (es->cursor_info.selection_offset > es->cursor_info.cursor_offset) {
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

internal void render_editor_hex_mode(Editor_State* es)
{
	glEnable(GL_SCISSOR_TEST);
	glScissor(es->container.minx, es->container.miny, es->container.maxx, es->container.maxy);

	// render text in the buffer
	if (es->render) {
		vec4 font_color = FONT_COLOR;

		// Setup the rendering info needed to render hex
		Font_RenderInInfo in_info = { 0 };
		Font_RenderOutInfo out_info = { 0 };
		{
			in_info.cursor_offset = -1;
			in_info.exit_on_max_width = true;
			in_info.max_width = es->container.maxx;
			in_info.seek_location = false;
			in_info.selection_offset = -1;
			es->cursor_info.this_line_count = -1;
			es->cursor_info.previous_line_count = -1;
			es->cursor_info.next_line_count = -1;

			if (es->cursor_info.handle_seek) {
				in_info.location_to_seek.x = es->cursor_info.seek_position.x;
				in_info.location_to_seek.y = es->cursor_info.seek_position.y;
				in_info.seek_location = true;
				es->cursor_info.handle_seek = false;
			}
		}

		const float left_text_padding = 20.0f;	// in pixels
		const float spacing = 4.0f;				// in pixels

		float offset_y = 0, offset_x = left_text_padding;
		int written = 0, num_bytes = 0, num_lines = 1, cursor_line = 0, selection_line = 0;
		int last_line_count = 0;
		int line_count = 0;

		while (num_bytes < _tm_valid_bytes[es->main_buffer_id]) {
			char hexbuffer[64];
			u64 num = *(es->buffer + num_bytes);
			int num_len = u8_to_str_base16(num, false, hexbuffer);

			if (num_bytes == es->cursor_info.cursor_offset) {
				in_info.cursor_offset = 0;
				cursor_line = num_lines;
			} else {
				in_info.cursor_offset = -1;
			}

			if (num_bytes <= es->cursor_info.selection_offset &&
				es->cursor_info.selection_offset != es->cursor_info.cursor_offset) {
				in_info.selection_offset = es->cursor_info.selection_offset - num_bytes;
				selection_line = num_lines;
			}
			else {
				in_info.selection_offset = -1;
			}

			written = prerender_text(es->container.minx + offset_x, es->container.maxy - font_rendering->max_height + offset_y,
				hexbuffer, num_len, &out_info, &in_info);

			// test seeking cursor from click
			if (out_info.seeked_index != -1) {
				es->cursor_info.cursor_offset = num_bytes;
			}

			if (out_info.exited_on_limit_width) {
				// set the count of characters rendered from the cursor previous, current and next lines
				if (cursor_line == num_lines) {
					es->cursor_info.this_line_count = line_count;
					es->cursor_info.previous_line_count = last_line_count;
					if(in_info.cursor_offset >= 0) es->cursor_info.cursor_column = in_info.cursor_offset;
				}
				if (cursor_line + 1 == num_lines) {
					es->cursor_info.next_line_count = line_count;
				}

				offset_y -= font_rendering->max_height;
				offset_x = left_text_padding;
				num_lines++;
				last_line_count = line_count;
				line_count = 0;
				continue;
			} else {
				line_count += written / 2;
				queue_text(es->container.minx + offset_x, es->container.maxy - font_rendering->max_height + offset_y, hexbuffer, num_len);
				offset_x = (out_info.exit_width - es->container.minx) + spacing;
			}
			es->cursor_info.cursor_line = cursor_line;

			if (es->selecting) {
				render_selection(es, num_lines + 1, num_bytes, written, &out_info);
			}

			num_bytes++;
			if (written == 0) break;	// if the space to render is too small for a single character than just leave
		}
		es->cursor_info.last_line = num_lines - 1;
		flush_text_batch(&font_color, num_bytes * 2);

		// render cursor overtop
		vec4 cursor_color = CURSOR_COLOR;
		float min_y = es->container.maxy - ((font_rendering->max_height) * (float)cursor_line) + font_rendering->descent;
		float max_y = es->container.maxy - ((font_rendering->max_height) * (float)(cursor_line - 1)) + font_rendering->descent;
		render_transparent_quad(out_info.cursor_minx, min_y, out_info.cursor_maxx, max_y, &cursor_color);

		// selection
		vec4 select_cursor_color = (vec4) { 0.7f, 0.9f, 0.85f, 0.5f };
		if (es->selecting && es->cursor_info.selection_offset != es->cursor_info.cursor_offset) {
			float min_selec_y = es->container.maxy - ((font_rendering->max_height) * (float)selection_line) + font_rendering->descent;
			float max_selec_y = es->container.maxy - ((font_rendering->max_height) * (float)(selection_line - 1)) + font_rendering->descent;
			render_transparent_quad(out_info.selection_minx, min_selec_y, out_info.selection_maxx, max_selec_y, &select_cursor_color);
		}

		glDisable(GL_SCISSOR_TEST);
	}
}

internal void render_editor_ascii_mode(Editor_State* es)
{
	glEnable(GL_SCISSOR_TEST);
	glScissor(es->container.minx, es->container.miny, es->container.maxx, es->container.maxy);

	if (es->render) {
		vec4 font_color = es->font_color;

		Font_RenderInInfo line_in_info = { 0 };
		Font_RenderOutInfo line_out_info = { 0 };

		Font_RenderInInfo in_info = { 0 };
		Font_RenderOutInfo out_info = { 0 };

		float min_height = es->container.miny;

		{
			in_info.cursor_offset = -1;
			in_info.exit_on_max_width = es->line_wrap;
			in_info.max_width = es->container.maxx;
			in_info.exit_on_line_feed = true;
			in_info.seek_location = false;
			in_info.selection_offset = -1;
			es->cursor_info.this_line_count = -1;
			es->cursor_info.previous_line_count = -1;
			es->cursor_info.next_line_count = -1;

			if (es->cursor_info.handle_seek) {
				in_info.location_to_seek.x = es->cursor_info.seek_position.x;
				in_info.location_to_seek.y = es->cursor_info.seek_position.y;
				in_info.seek_location = true;
				es->cursor_info.handle_seek = false;
			}
		}
		{
			line_in_info.cursor_offset = -1;
			line_in_info.exit_on_max_width = false;
			line_in_info.max_width = es->container.maxx;
			line_in_info.exit_on_line_feed = false;
			line_in_info.seek_location = false;
			line_in_info.selection_offset = -1;
			es->cursor_info.this_line_count = -1;
			es->cursor_info.previous_line_count = -1;
			es->cursor_info.next_line_count = -1;
		}
		float line_number_width = 0.0f;

		int written = 0, num_bytes = 0, cursor_line = 0, num_lines = 1, selection_line = 0;
		float offset_y = 0, offset_x = 0;
		int last_line_count = 0;
		int cursor_location = -1;
		bool exited_on_limit_height = false;

		if (es->update_line_number) {
			es->first_line_number = get_cursor_info(es->main_buffer_id, es->cursor_info.block_offset).line_number.lf;
			es->update_line_number = false;
		}
		s64 aux_line_number = es->first_line_number;

		while (num_bytes < es->buffer_valid_bytes - es->cursor_info.block_offset) {
			if (num_bytes <= CURSOR_RELATIVE_OFFSET) {
				in_info.cursor_offset = CURSOR_RELATIVE_OFFSET - num_bytes;
				cursor_line = num_lines;
			} else {
				in_info.cursor_offset = -1;
			}

			if (num_bytes <= SELECTION_RELATIVE_OFFSET &&
				es->cursor_info.selection_offset != es->cursor_info.cursor_offset) {
				in_info.selection_offset = SELECTION_RELATIVE_OFFSET - num_bytes;
				selection_line = num_lines;
			} else {
				in_info.selection_offset = -1;
			}

			char line_buffer[64] = { 0 };
			// the line number of the first line in this view buffer
			int ln_count = s64_to_str_base10(aux_line_number, line_buffer);
			float line_number_length = font_rendering->max_width * 4.0f;// line_out_info.exit_width - es->container.minx; //  @temporary
			if (es->render_line_numbers) {
				vec4 line_number_color = CURSOR_COLOR;
				render_text(es->container.minx, es->container.maxy - font_rendering->max_height + offset_y, line_buffer,
					ln_count, &line_number_color);
			} else {
				line_number_length = 0.0f;
			}

			s64 num_to_write = es->buffer_size - num_bytes - (es->buffer_size - es->buffer_valid_bytes);
			written = prerender_text(es->container.minx + line_number_length, es->container.maxy - font_rendering->max_height + offset_y,
									 es->buffer + num_bytes, num_to_write, &out_info, &in_info);
			aux_line_number++;
			line_number_width = es->container.minx + line_number_length - 4.0f - font_rendering->max_width;

			if (out_info.seeked_index != -1) {
				// test seeking cursor from click
				es->cursor_info.cursor_offset = es->cursor_info.block_offset + num_bytes + out_info.seeked_index;
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

			queue_text(es->container.minx + line_number_length, es->container.maxy - font_rendering->max_height + offset_y, es->buffer + num_bytes, written);

			// set the count of characters rendered from the cursor previous, current and next lines
			if (cursor_line == num_lines) {
				es->cursor_info.this_line_count = written;
				es->cursor_info.previous_line_count = last_line_count;
				es->cursor_info.cursor_column = written - (num_bytes + written - CURSOR_RELATIVE_OFFSET);
			}
			if (cursor_line + 1 == num_lines) {
				es->cursor_info.next_line_count = MAX(1, written);
			}

			if (num_lines == 1) {
				es->first_line_count = written;
			}

			offset_y -= font_rendering->max_height;
			offset_x = 0.0f;
			num_lines++;
			last_line_count = written;

			//if (written > max_in_a_line) max_in_a_line = written;
			num_bytes += written;

			if (es->selecting) {
				render_selection(es, num_lines, num_bytes, written, &out_info);
			}

			if (es->container.maxy - font_rendering->max_height + offset_y < min_height) {
				exited_on_limit_height = true;
				break;
			}

			if (written == 0) break;	// if the space to render is too small for a single character than just leave
		}

		if (cursor_line == 0) cursor_line = 1;
		es->last_line_count = last_line_count;
		es->cursor_info.last_line = num_lines - 1;
		es->cursor_info.cursor_line = cursor_line;
		flush_text_batch(&font_color, num_bytes);

		// render cursor overtop
		//if (cursor_location != 0) return;
		//vec4 cursor_color = CURSOR_COLOR;
		float min_y = es->container.maxy - ((font_rendering->max_height) * (float)cursor_line) + font_rendering->descent;
		float max_y = es->container.maxy - ((font_rendering->max_height) * (float)(cursor_line - 1)) + font_rendering->descent;
		float min_x = MAX(out_info.cursor_minx, es->container.minx);

		float max_x = min_x + 1.0f;
		//float max_x = out_info.cursor_maxx;

		render_transparent_quad(min_x, min_y, max_x, max_y, &es->cursor_color);
		vec4 vertical_line_color = es->cursor_color;//add_v4(es->cursor_color, (vec4) { 0.0f, 0.0f, 0.0f, -0.5f });
		render_transparent_quad(line_number_width, es->container.miny, line_number_width + 1.0f, es->container.maxy,
			&vertical_line_color);

		// selection
		vec4 select_cursor_color = (vec4) { 0.7f, 0.9f, 0.85f, 0.5f };
		if (es->selecting && es->cursor_info.selection_offset != es->cursor_info.cursor_offset) {
			float min_selec_y = es->container.maxy - ((font_rendering->max_height) * (float)selection_line) + font_rendering->descent;
			float max_selec_y = es->container.maxy - ((font_rendering->max_height) * (float)(selection_line - 1)) + font_rendering->descent;
			render_transparent_quad(out_info.selection_minx, min_selec_y, out_info.selection_maxx, max_selec_y, &select_cursor_color);
		}
	}

	glDisable(GL_SCISSOR_TEST);
}

internal void render_editor_binary_mode(Editor_State* es)
{
	// @TODO: Render binary mode.
}

void editor_start_selection(Editor_State* es) {
	if (!es->selecting) {
		es->cursor_info.selection_offset = es->cursor_info.cursor_offset;
	}
	es->selecting = true;
}

void editor_end_selection(Editor_State* es) {
	if(es->selecting) es->selecting = true;
}

void editor_reset_selection(Editor_State* es){
	es->selecting = false;
}

void render_editor(Editor_State* es)
{
	switch (es->mode) {
		case EDITOR_MODE_ASCII: {
			render_editor_ascii_mode(es);
		} break;
		case EDITOR_MODE_HEX: {
			render_editor_hex_mode(es);
		} break;
		case EDITOR_MODE_BINARY: {
			render_editor_binary_mode(es);
		} break;
	}

	es->cursor_info.handle_seek = false;
}

internal Editor_Mode next_mode(Editor_State* es) {
	int mode = es->mode;
	mode++;
	if (mode >= EDITOR_MODE_END) mode = 0;
	return mode;
}

void update_buffer(Editor_State* es) {
	setup_view_buffer(es, 0, SCREEN_BUFFER_SIZE, true);
	es->cursor_info.cursor_offset = 0;
	es->cursor_info.block_offset = 0;
}

internal void scroll_down_ascii(Editor_State* es) {
	setup_view_buffer(es, es->cursor_info.block_offset + es->first_line_count, SCREEN_BUFFER_SIZE, false);
	es->cursor_info.block_offset += es->first_line_count;
	es->update_line_number = true;
}

internal void scroll_up_ascii(Editor_State* es, s64 new_line_count) {
	setup_view_buffer(es, es->cursor_info.block_offset - new_line_count, SCREEN_BUFFER_SIZE, false);
	es->cursor_info.block_offset -= new_line_count;
	es->update_line_number = true;
}

internal void editor_handle_key_down_ascii(Editor_State* es, s32 key, bool selection_reset) {
	s64 cursor = es->cursor_info.cursor_offset;

	cursor_info cinfo;
	if (key == VK_UP || key == VK_DOWN || key == VK_LEFT || key == VK_RIGHT || key == VK_HOME || key == VK_END)
	{
		// selection_stuff
		if (keyboard_state.key[VK_SHIFT]) editor_start_selection(es);
		else if (selection_reset) return;
	}

	if (key == VK_RIGHT) {
		s64 increment = 1;
		if (keyboard_state.key[17]) increment = 8;

		if (es->selecting && es->cursor_info.cursor_offset == es->cursor_info.selection_offset) {
			es->cursor_info.selection_offset = es->cursor_info.cursor_offset;
		}
		es->cursor_info.cursor_snaped_column = es->cursor_info.cursor_column + increment;
		if (es->cursor_info.cursor_snaped_column >= es->cursor_info.this_line_count) {
			es->cursor_info.cursor_snaped_column = es->cursor_info.cursor_snaped_column - es->cursor_info.this_line_count;
		}
		es->cursor_info.cursor_offset = MIN(es->cursor_info.cursor_offset + increment, es->buffer_valid_bytes + es->cursor_info.block_offset);
		if (es->cursor_info.cursor_offset == es->buffer_valid_bytes + es->cursor_info.block_offset) return;	// dont pass the size of buffer
		if (es->cursor_info.cursor_line == es->cursor_info.last_line) {
			if (es->cursor_info.this_line_count - 1 == es->cursor_info.cursor_column) {
				// this should be the last character on screen so scroll down
				scroll_down_ascii(es);
			}
		}
	}
	if (key == VK_LEFT) {
		s64 increment = 1;
		if (keyboard_state.key[17]) increment = 8;

		if (es->selecting && es->cursor_info.cursor_offset == es->cursor_info.selection_offset) {
			es->cursor_info.selection_offset = es->cursor_info.cursor_offset;
		}
		es->cursor_info.cursor_snaped_column = es->cursor_info.cursor_column - increment;
		if (es->cursor_info.cursor_snaped_column < 0) {
			s64 new_snap = es->cursor_info.previous_line_count + es->cursor_info.cursor_snaped_column;
			es->cursor_info.cursor_snaped_column = new_snap;
		}

		if (CURSOR_RELATIVE_OFFSET - increment < 0){
			// go back one line on the view
			u64 first_char_pos = es->cursor_info.cursor_offset - es->cursor_info.cursor_column;
			if (first_char_pos == 0) return;	// if this is the first character, no need to go left
			cinfo = get_cursor_info(es->main_buffer_id, first_char_pos - 1);
			s64 amount_last_line = (first_char_pos - 1) - cinfo.previous_line_break.lf;

			s64 back_amt = MAX(0, es->cursor_info.cursor_offset - increment);
			es->cursor_info.cursor_offset = back_amt;

			scroll_up_ascii(es, amount_last_line);
		} else {
			// go back normally because the line is in view
			es->cursor_info.cursor_offset = MAX(es->cursor_info.cursor_offset - increment, 0);
		}
	}

	if (key == VK_HOME) {
		es->cursor_info.cursor_offset -= es->cursor_info.cursor_column;
		es->cursor_info.cursor_snaped_column = 0;
	}
	if (key == VK_END) {
		s64 value = es->cursor_info.this_line_count - es->cursor_info.cursor_column;
		s64 is_final = 0;
		if (es->cursor_info.cursor_offset + value < es->buffer_size) {
			value--;
			is_final++;
		}
		es->cursor_info.cursor_offset += value;
		es->cursor_info.cursor_snaped_column = es->cursor_info.this_line_count + is_final;
	}

	if (!es->is_block_text) return;

	if (key == VK_UP) {
		int snap = es->cursor_info.cursor_snaped_column;
		s64 back_amt = es->cursor_info.cursor_offset - es->cursor_info.cursor_column - 1;

		if (back_amt < 0) return;	// this is the first line in the text, no need to go up

		cinfo = get_cursor_info(es->main_buffer_id, es->cursor_info.cursor_offset - es->cursor_info.cursor_column - 1);
		s64 previous_line_count = (es->cursor_info.cursor_offset - es->cursor_info.cursor_column - 1) - cinfo.previous_line_break.lf;

		int c = previous_line_count - MAX(snap, es->cursor_info.cursor_column - 1);
		if (c <= 0) c = 1;
		c += es->cursor_info.cursor_column;

		// if the cursor is on the first line
		if (CURSOR_RELATIVE_OFFSET - es->cursor_info.cursor_column - 1 < 0) {
			assert(es->cursor_info.cursor_line == 1);
			scroll_up_ascii(es, previous_line_count);
		}

		es->cursor_info.cursor_offset -= c;
	}
	if (key == VK_DOWN) {
		if (es->line_wrap) {
			// todo
		} else {
			s64 count_from_cursor_to_next_lf = get_cursor_info(es->main_buffer_id, es->cursor_info.cursor_offset).next_line_break.lf;
			if (count_from_cursor_to_next_lf == -1) {
				// if we are at the last line in the text
				return;
			}
			count_from_cursor_to_next_lf -= es->cursor_info.cursor_offset;

			s64 count_of_next_line = get_cursor_info(es->main_buffer_id, es->cursor_info.cursor_offset + count_from_cursor_to_next_lf + 1).next_line_break.lf;
			if (count_of_next_line == -1) {
				// if we are at the penultima line we won't have a \n at the end of the text
				count_of_next_line = _tm_text_size[es->main_buffer_id] - (es->cursor_info.cursor_offset + count_from_cursor_to_next_lf + 1);

			} else {
				// otherwise proceed normally
				count_of_next_line -= es->cursor_info.cursor_offset + count_from_cursor_to_next_lf + 1;
			}

			s64 cursor_column = es->cursor_info.cursor_column;
			s64 snap = es->cursor_info.cursor_snaped_column;

			s64 count_to_skip = MIN(MAX(cursor_column, snap) + count_from_cursor_to_next_lf + 1, count_from_cursor_to_next_lf + 1 + count_of_next_line);

			if (CURSOR_RELATIVE_OFFSET + count_to_skip <= es->buffer_size && es->cursor_info.next_line_count > 0) {
				// case in which we are inside the area of rendering
				es->cursor_info.cursor_offset += count_to_skip;
			} else {
				// the next line is outside the view of the window
				es->cursor_info.cursor_offset += count_to_skip;
				scroll_down_ascii(es);
			}
		}
	}
}

void editor_handle_key_down(Editor_State* es, s32 key)
{
	bool selection_reset = false;

	if ((key == VK_LEFT || key == VK_RIGHT || key == VK_UP || key == VK_DOWN || key == VK_END || key == VK_HOME)
		&& !keyboard_state.key[VK_SHIFT] && key != BACKSPACE_KEY && !keyboard_state.key[BACKSPACE_KEY]) {
		if (!keyboard_state.key[CTRL_KEY]) {
			if(es->selecting) selection_reset = true;
			es->selecting = false;
		}
	}

	if (key == 'R' && keyboard_state.key[17]) { recompile_font_shader(); return; }
	if (key == 'P' && keyboard_state.key[17]) {
		es->mode = next_mode(es);
		es->cursor_info.cursor_snaped_column = 0;
	}

	editor_handle_key_down_ascii(es, key, selection_reset);
}

void editor_handle_lmouse_down(Editor_State* es, int x, int y)
{
	float xf = (float)x;
	float yf = (float)y;

	yf = win_state.win_height - yf;

	es->cursor_info.handle_seek = true;
	es->cursor_info.seek_position.x = xf;
	es->cursor_info.seek_position.y = yf;
}
