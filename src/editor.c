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

extern Window_State win_state;

#define CURSOR_DELAY 0.5f

internal double cursor_time_reference = 0;
internal bool b_render_cursor = false;

#define CURSOR_RELATIVE_OFFSET (es->cursor_info.cursor_offset - es->cursor_info.block_offset)
#define SELECTION_RELATIVE_OFFSET (es->cursor_info.selection_offset - es->cursor_info.block_offset)

void init_editor_state(Editor_State* es)
{
	es->cursor_info.cursor_offset = 0;
	es->cursor_info.cursor_column = 0;
	es->cursor_info.cursor_snaped_column = 0;
	es->cursor_info.previous_line_count = 0;
	es->cursor_info.next_line_count = 0;
	es->cursor_info.this_line_count = 0;
	es->cursor_info.cursor_line = 0;
	es->cursor_info.block_offset = 0;
	es->selecting = false;
	es->font_color = (vec4) { 0, 0, 0, 0 };
	es->cursor_color = (vec4) { 0, 0, 0, 0 };
	es->line_number_color = (vec4) { 0, 0, 0, 0 };

	es->render = true;
	es->debug = true;
	es->line_wrap = false;
	es->mode = EDITOR_MODE_ASCII;
	es->render_line_numbers = false;
	es->show_cursor = false;
	es->references = 1;

	es->cursor_info.handle_seek = false;
	es->individual_char_handler = null;

	es->parent_panel = null;
}

void setup_view_buffer(Editor_State* es, s64 offset, s64 size, bool force_loading) {
	if (offset < get_tid_text_size(es->main_buffer_tid) && !force_loading) {
		set_cursor_begin(es->main_buffer_tid, offset);
	} else {
		es->buffer = get_text_buffer(es->main_buffer_tid, size, offset);
		es->buffer_valid_bytes = get_tid_valid_bytes(es->main_buffer_tid);
	}
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
	float min_y = es->container.maxy - ((font_rendering->max_height) * (float)(num_lines - 1 + 1)) + font_rendering->descent;
	float max_y = es->container.maxy - ((font_rendering->max_height) * (float)(num_lines - 2 + 1)) + font_rendering->descent;
	float max_x = 0.0f;
	float min_x = 0.0f;


	// if the selection is happening with the selection cursor back
	if (es->cursor_info.selection_offset < es->cursor_info.cursor_offset) {
		int line_count = CURSOR_RELATIVE_OFFSET - (num_bytes - line_written);
		int selec_count = SELECTION_RELATIVE_OFFSET - (num_bytes - line_written);

		bool is_cursor_in_this_line = (line_count < line_written && line_count >= 0) ? true : false;
		bool is_selection_in_this_line = (selec_count < line_written && selec_count >= 0) ? true : false;

		if (line_written <= MIN(line_count, selec_count) && !is_cursor_in_this_line && !is_selection_in_this_line) return;

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

internal void fill_render_info(Editor_State* es, Font_RenderInInfo* in_info, Font_RenderOutInfo* out_info) {
	in_info->cursor_relative_offset = CURSOR_RELATIVE_OFFSET;
	in_info->exit_on_line_feed = (es->mode == EDITOR_MODE_ASCII) ? true : false;
	in_info->exit_on_max_width = (es->line_wrap) ? true : false;
	in_info->location_to_seek = es->cursor_info.seek_position;
	in_info->max_width = es->container.maxx;
	in_info->seek_location = (es->cursor_info.handle_seek) ? true : false;
	in_info->selection_relative_offset = SELECTION_RELATIVE_OFFSET;

	es->cursor_info.this_line_count = -1;
	es->cursor_info.next_line_count = -1;
	es->cursor_info.previous_line_count = -1;
	es->cursor_info.handle_seek = false;
	es->exited_on_limit_height = false;
}

internal void render_cursor(Editor_State* es, s32 cursor_line, Font_RenderOutInfo* out_info) {
	double this_time = get_time();
	if ((this_time - cursor_time_reference) > CURSOR_DELAY)
	{
		cursor_time_reference = this_time;
		b_render_cursor = !b_render_cursor;
	}

	if (b_render_cursor)
	{
		float min_y = es->container.maxy - ((font_rendering->max_height) * (float)(cursor_line + 1)) + font_rendering->descent;
		float max_y = es->container.maxy - ((font_rendering->max_height) * (float)(cursor_line + 0)) + font_rendering->descent;
		float min_x = MAX(out_info->cursor_minx, es->container.minx);
		float max_x = min_x + 1.0f;
		render_transparent_quad(min_x, min_y, max_x, max_y, &es->cursor_color);
	}
}

internal void render_selection_cursor(Editor_State* es, s32 selection_line, Font_RenderOutInfo* out_info) {
	vec4 select_cursor_color = (vec4) { 0.7f, 0.9f, 0.85f, 0.5f };
	if (es->selecting && es->cursor_info.selection_offset != es->cursor_info.cursor_offset) {
		float min_selec_y = es->container.maxy - ((font_rendering->max_height) * (float)(selection_line + 1)) + font_rendering->descent;
		float max_selec_y = es->container.maxy - ((font_rendering->max_height) * (float)(selection_line - 1 + 1)) + font_rendering->descent;
		render_transparent_quad(out_info->selection_minx, min_selec_y, out_info->selection_maxx, max_selec_y, &select_cursor_color);
	}
}

internal void update_line_number(Editor_State* es) {
	if (es->update_line_number) {
		es->first_line_number = get_cursor_info(es->main_buffer_tid, es->cursor_info.block_offset, true, false, false).line_number.lf;
		es->update_line_number = false;
	}
}

internal void update_and_render_editor_hex_mode(Editor_State* es) {
	Font_RenderInInfo in_info;
	Font_RenderOutInfo out_info;
	fill_render_info(es, &in_info, &out_info);
	es->line_wrap = true;
	
	s64 bytes_to_render = get_tid_valid_bytes(es->main_buffer_tid) - es->cursor_info.block_offset;
	s64 bytes_rendered = 0;
	float offset_x = 0.0f, offset_y = 0.0f;
	s32 num_lines = 0;
	s32 cursor_line = -1, selection_line = 0;
	u8* buffer_ptr = es->buffer + es->cursor_info.block_offset;

	bool exited_on_limit_height = false;
	update_line_number(es);
	s32 absolute_line_number = es->first_line_number;
	s64 bytes_lines_rendered = 0;
	float vertical_line_width = (es->render_line_numbers) ? font_rendering->max_width * 10.0f : 0.0f;

	if (es->main_buffer_tid.is_block_text) {
		bytes_to_render = get_tid_valid_bytes(es->main_buffer_tid);
	}

	if (bytes_to_render == 0) {
		buffer_ptr = es->buffer;
		buffer_ptr[0] = ' ';
		bytes_to_render = 1;
	}

	float posx = 0.0f, posy = 0.0f;
	int column = 0;
	int prev_line_bytes_count = 0;

	if (es->render_line_numbers) in_info.max_width -= font_rendering->max_width;

	if (es->render) {
		while (bytes_to_render > 0) {
			posx = es->container.minx + offset_x;
			posy = es->container.maxy - font_rendering->max_height + offset_y;
			buffer_ptr = es->buffer + bytes_rendered;

			// update cursor in_info in order to render correctly
			if (bytes_rendered <= CURSOR_RELATIVE_OFFSET) {
				in_info.cursor_relative_offset = CURSOR_RELATIVE_OFFSET - bytes_rendered;
			} else {
				in_info.cursor_relative_offset = -1;
			}
			if (bytes_rendered <= SELECTION_RELATIVE_OFFSET && es->cursor_info.selection_offset != es->cursor_info.cursor_offset) {
				in_info.selection_relative_offset = SELECTION_RELATIVE_OFFSET - bytes_rendered;
				selection_line = num_lines;
			} else {
				in_info.selection_relative_offset = -1;
			}

			// vertical line rendering
			if (es->render_line_numbers && column == 0) {
				u8 line_buffer[64] = { 0 };
				s32 num_length = u64_to_str_base10(es->cursor_info.block_offset + bytes_rendered, line_buffer);
				//u64_to_str_base16(es->cursor_info.block_offset + bytes_rendered, true, line_buffer);

				queue_text(posx, posy, line_buffer, num_length, 1);
				bytes_lines_rendered += num_length;

				posx += vertical_line_width;
			}

			u8 n_buffer[64] = { 0 };
			s64 len = u8_to_str_base16(buffer_ptr[0], false, n_buffer);
			s32 bytes_written = prerender_text(posx, posy, n_buffer, len, &out_info, &in_info);
			queue_text(posx, posy, n_buffer, len, 0);

			if (out_info.seeked_index != -1) {
				// test seeking cursor from click
				es->cursor_info.cursor_offset = es->cursor_info.block_offset + bytes_rendered + out_info.seeked_index;
				es->cursor_info.cursor_snaped_column = out_info.seeked_index;
			}

			bytes_to_render -= 1;
			bytes_rendered += 1;

			if (bytes_to_render <= 0 && cursor_line == -1) {
				out_info.found_cursor = true;
			}

			// update count of characters in each line
			if (out_info.found_cursor) {
				cursor_line = num_lines;
				es->cursor_info.cursor_column = column;
			}

			column += 1;

			if (out_info.exited_on_limit_width || bytes_to_render <= 0) {
				// set the count of characters rendered from the cursor previous, current and next lines
				offset_y -= font_rendering->max_height;
				offset_x = 0.0f;
				prev_line_bytes_count += 1;
				
				column = 0;

				if (num_lines == 0) {
					es->first_line_count = prev_line_bytes_count;
				}
				if (cursor_line == num_lines) {
					es->cursor_info.this_line_count = prev_line_bytes_count;
				}
				else if (cursor_line == -1) {
					es->cursor_info.previous_line_count = prev_line_bytes_count;
				}
				else if (cursor_line == num_lines - 1) {
					es->cursor_info.next_line_count = prev_line_bytes_count;
				}

				prev_line_bytes_count = 0;
				num_lines += 1;
			} else {
				prev_line_bytes_count += 1;
				const float spacing = 4.0f;
				offset_x = (out_info.exit_width - es->container.minx) + spacing;
			}

			es->cursor_info.last_line = num_lines - 1;

			if (es->selecting) {
				render_selection(es, num_lines + 1, bytes_rendered, bytes_written, &out_info);
			}

			if (es->container.maxy - font_rendering->max_height + offset_y < es->container.miny) {
				exited_on_limit_height = true;
				es->exited_on_limit_height = true;
				break;
			}
			if (bytes_to_render == 0) {
				if (!out_info.exited_on_limit_width && !out_info.exited_on_line_feed) continue;
				buffer_ptr[bytes_written] = ' ';
				bytes_to_render = 1;
			}
		}
		if (cursor_line == -1) {
			cursor_line = num_lines - 1;
			if (out_info.exited_on_line_feed) {
				cursor_line += 1;
				es->cursor_info.cursor_column = 0;
			}
		}
		if (es->cursor_info.cursor_offset == get_tid_text_size(es->main_buffer_tid)) es->cursor_info.cursor_column += 1;

		get_spare_lines(es);
		es->cursor_info.cursor_line = cursor_line;

		flush_text_batch(&es->cursor_color, bytes_lines_rendered, 1);
		render_selection_cursor(es, selection_line, &out_info);

		flush_text_batch(&es->font_color, bytes_rendered * 2, 0);

		if (es->show_cursor) render_cursor(es, cursor_line, &out_info);
	}
}

internal void update_and_render_editor_ascii_mode(Editor_State* es) {
	vec4 font_color = es->font_color;

	Font_RenderInInfo in_info;
	Font_RenderOutInfo out_info;

	fill_render_info(es, &in_info, &out_info);

	s64 bytes_to_render = es->buffer_valid_bytes - es->cursor_info.block_offset;
	s64 bytes_rendered = 0;
	s64 bytes_lines_rendered = 0;
	float offset_x = 0, offset_y = 0;
	s32 num_lines = 0;
	s32 cursor_line = -1, selection_line = 0;
	u8* buffer_ptr = es->buffer + es->cursor_info.block_offset;
	float vertical_line_width = (es->render_line_numbers) ? font_rendering->max_width * 4.0f : 0.0f;

	bool exited_on_limit_height = false;

	update_line_number(es);
	s32 absolute_line_number = es->first_line_number;

	if (es->main_buffer_tid.is_block_text) {
		bytes_to_render = get_tid_valid_bytes(es->main_buffer_tid);
	}

	if (bytes_to_render == 0) {
		buffer_ptr = es->buffer;
		buffer_ptr[0] = ' ';
		bytes_to_render = 1;
	}

	if (es->render) {
		while (bytes_to_render > 0){
			float posx = es->container.minx;
			float posy = es->container.maxy - font_rendering->max_height + offset_y;
			buffer_ptr = es->buffer + bytes_rendered;

			// update cursor in_info in order to render correctly
			if (bytes_rendered <= CURSOR_RELATIVE_OFFSET) {
				in_info.cursor_relative_offset = CURSOR_RELATIVE_OFFSET - bytes_rendered;
			} else {
				in_info.cursor_relative_offset = -1;
			}
			if (bytes_rendered <= SELECTION_RELATIVE_OFFSET && es->cursor_info.selection_offset != es->cursor_info.cursor_offset) {
				in_info.selection_relative_offset = SELECTION_RELATIVE_OFFSET - bytes_rendered;
				selection_line = num_lines;
			} else {
				in_info.selection_relative_offset = -1;
			}

			// vertical line rendering
			if (es->render_line_numbers) {
				u8 line_buffer[64] = { 0 };
				s32 num_length = s64_to_str_base10(absolute_line_number, line_buffer);

				queue_text(posx, posy, line_buffer, num_length, 1);
				bytes_lines_rendered += num_length;

				absolute_line_number += 1;
				posx += vertical_line_width;
			}

			// prerender text to know its positioning
			s32 bytes_written = prerender_text(posx, posy, buffer_ptr, bytes_to_render, &out_info, &in_info);
			queue_text(posx, posy, buffer_ptr, bytes_written, 0);

			if (out_info.seeked_index != -1) {
				// test seeking cursor from click
				es->cursor_info.cursor_offset = es->cursor_info.block_offset + bytes_rendered + out_info.seeked_index;
				es->cursor_info.cursor_snaped_column = out_info.seeked_index;
			}

			{
				// after prerender
				bytes_rendered += bytes_written;
				bytes_to_render -= bytes_written;
				offset_y -= font_rendering->max_height;
				offset_x = 0.0f;

				if (bytes_to_render <= 0 && cursor_line == -1) {
					out_info.found_cursor = true;
				}

				// update count of characters in each line
				if (out_info.found_cursor) cursor_line = num_lines;
				if (num_lines == 0) {
					es->first_line_count = bytes_written;
				}
				if (cursor_line == num_lines) {
					es->cursor_info.this_line_count = bytes_written;
					es->cursor_info.cursor_column = in_info.cursor_relative_offset;
				} else if (cursor_line == -1) {
					es->cursor_info.previous_line_count = bytes_written;
				} else if (cursor_line == num_lines - 1) {
					es->cursor_info.next_line_count = bytes_written;
				}
				num_lines += 1;
			}

			es->cursor_info.last_line = num_lines - 1;

			if (es->selecting) render_selection(es, num_lines, bytes_rendered, bytes_written, &out_info);

			// if exceeding height, prerender next and quit
			if (es->container.maxy - font_rendering->max_height + offset_y < es->container.miny) {
				exited_on_limit_height = true;
				es->exited_on_limit_height = true;
				break;
			}
			if (bytes_to_render == 0) {
				if (!out_info.exited_on_limit_width && !out_info.exited_on_line_feed) continue;
				buffer_ptr[bytes_written] = ' ';
				bytes_to_render = 1;
			}
		}
		if (cursor_line == -1) {
			cursor_line = num_lines - 1;
			if (out_info.exited_on_line_feed) {
				cursor_line += 1;
				es->cursor_info.cursor_column = 0;
			}
		}
		get_spare_lines(es);

		glEnable(GL_SCISSOR_TEST);
		glScissor(es->container.minx, es->container.miny, es->container.maxx, es->container.maxy);

		es->cursor_info.cursor_line = cursor_line;
		flush_text_batch(&font_color, bytes_rendered, 0);

		flush_text_batch(&es->cursor_color, bytes_lines_rendered, 1);	// @temporary color this is the line separating numbers and text

		if (es->show_cursor) render_cursor(es, cursor_line, &out_info);
		render_selection_cursor(es, selection_line, &out_info);

		if (es->render_line_numbers) {
			vertical_line_width += es->container.minx - 5.0f;
			render_transparent_quad(vertical_line_width, es->container.miny, vertical_line_width + 1.0f, es->container.maxy, &es->line_number_color);
		}
		glDisable(GL_SCISSOR_TEST);
	}
}

internal void update_and_render_editor_binary_mode(Editor_State* es)
{
	// @TODO: Update and Render binary mode.
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

void update_and_render_editor(Editor_State* es)
{
	es->buffer_valid_bytes = get_tid_valid_bytes(es->main_buffer_tid);

	switch (es->mode) {
		case EDITOR_MODE_ASCII: {
			update_and_render_editor_ascii_mode(es);
		} break;
		case EDITOR_MODE_HEX: {
			update_and_render_editor_hex_mode(es);
		} break;
		case EDITOR_MODE_BINARY: {
			update_and_render_editor_binary_mode(es);
		} break;
	}

	es->cursor_info.handle_seek = false;
}

Editor_Mode next_mode(Editor_State* es) {
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

internal void scroll_down_ascii(Editor_State* es, s64 offset) {
	setup_view_buffer(es, es->cursor_info.block_offset + offset, SCREEN_BUFFER_SIZE, false);
	es->cursor_info.block_offset += offset;
	es->update_line_number = true;
}

internal void scroll_up_ascii(Editor_State* es, s64 new_line_count) {
	setup_view_buffer(es, es->cursor_info.block_offset - new_line_count, SCREEN_BUFFER_SIZE, false);
	es->cursor_info.block_offset -= new_line_count;
	es->update_line_number = true;
}

internal s32 get_spare_lines(Editor_State* es) {
	float size = es->container.maxy - es->container.miny;
	s32 fitting = size / font_rendering->max_height;

	s32 res = fitting - es->cursor_info.last_line;
	return MAX(0, res - 1);
}

void change_mode(Editor_State* es, Editor_Mode mode) {
	es->mode = mode;
	if (mode == EDITOR_MODE_ASCII) {
		es->line_wrap = false;
	} else if (mode == EDITOR_MODE_HEX) {
		es->line_wrap = true;
	}

}

#define KEY_LEFT_CTRL 17

void cursor_force(Editor_State* es, s64 pos) {

	if (es->cursor_info.cursor_offset > pos) {
		// go back
		s64 decr = es->cursor_info.cursor_offset - pos;
		cursor_left(es, decr);
	} else if(es->cursor_info.cursor_offset == pos){
		return;
	} else {
		// go forward
		s64 incr = pos - es->cursor_info.cursor_offset;
		cursor_right(es, incr);
	}
}

void cursor_left(Editor_State* es, s64 decrement) {
	cursor_info cinfo;
	if (decrement <= 0) return;

	if (es->cursor_info.cursor_offset == 0) return;

	if (es->mode == EDITOR_MODE_HEX) {
		s64 rel_cursor = CURSOR_RELATIVE_OFFSET - decrement;
		if (rel_cursor < 0) {
			// need to scroll up
			s64 max_count = MAX(MAX(es->cursor_info.this_line_count, es->cursor_info.next_line_count), es->cursor_info.previous_line_count);
			s64 nlines = (s64)ceilf(-((float)rel_cursor) / max_count);
			scroll_up_ascii(es, max_count * nlines);
			es->cursor_info.cursor_offset -= decrement;
		} else {
			es->cursor_info.cursor_offset -= decrement;
		}
	}
	else if (es->mode == EDITOR_MODE_ASCII) {
		// snap cursor logic
		es->cursor_info.cursor_snaped_column = MIN(0, es->cursor_info.cursor_column - decrement);
		if (es->cursor_info.cursor_snaped_column < 0) {
			s64 new_snap = es->cursor_info.previous_line_count + es->cursor_info.cursor_snaped_column;
			es->cursor_info.cursor_snaped_column = new_snap;
		}

		cinfo = get_cursor_info(es->main_buffer_tid, MAX(es->cursor_info.cursor_offset - decrement, 0), false, true, false);
		s64 rel_cursor = CURSOR_RELATIVE_OFFSET - decrement;

		s64 count_previous_lf = MAX(es->cursor_info.cursor_offset - decrement, 0) - cinfo.previous_line_break.lf - 1;
		if (es->cursor_info.cursor_offset - decrement - count_previous_lf < 0) {
			decrement = es->cursor_info.cursor_offset - count_previous_lf;
		}
		if (rel_cursor < 0) {

			scroll_up_ascii(es, decrement + count_previous_lf - CURSOR_RELATIVE_OFFSET);
			es->cursor_info.cursor_offset -= decrement;
			es->cursor_info.cursor_offset = MAX(0, es->cursor_info.cursor_offset);
		}
		else {
			es->cursor_info.cursor_offset = MAX(es->cursor_info.cursor_offset - decrement, 0);
		}
	}
}

void cursor_right(Editor_State* es, s64 increment) {
	increment = MAX(0, MIN(get_tid_text_size(es->main_buffer_tid) - es->cursor_info.cursor_offset, increment));
	if (increment == 0) return;

	if (es->mode == EDITOR_MODE_ASCII) {
		// snap cursor logic
		es->cursor_info.cursor_snaped_column = es->cursor_info.cursor_column + increment;
		if (es->cursor_info.cursor_snaped_column > es->cursor_info.this_line_count) {
			es->cursor_info.cursor_snaped_column = es->cursor_info.cursor_snaped_column - es->cursor_info.this_line_count;
		}

		s64 lines_between_cursor_and_endline = es->cursor_info.last_line - es->cursor_info.cursor_line;

		s64 ln_start = get_cursor_info(es->main_buffer_tid, es->cursor_info.cursor_offset, true, true, false).line_number.lf;
		s64 ln_end = get_cursor_info(es->main_buffer_tid, es->cursor_info.cursor_offset + increment, true, true, false).line_number.lf;
		s64 num_of_lf_inside_increment = ln_end - ln_start;

		s64 lines_to_vanish = num_of_lf_inside_increment - lines_between_cursor_and_endline;

		// here we have to do
		lines_to_vanish -= get_spare_lines(es);

		if (lines_to_vanish <= 0) {
			es->cursor_info.cursor_offset = MIN(es->cursor_info.cursor_offset + increment, get_tid_text_size(es->main_buffer_tid));
			return;
		}

		s64 num_chars_to_vanish = 0;
		s64 aux_cursor = es->cursor_info.block_offset;
		for (s64 i = 0; i < lines_to_vanish; ++i) {
			s64 aux = get_cursor_info(es->main_buffer_tid, aux_cursor, false, true, false).next_line_break.lf - aux_cursor + 1;
			aux_cursor += aux;
			num_chars_to_vanish += aux;
		}
		scroll_down_ascii(es, num_chars_to_vanish);

		es->cursor_info.cursor_offset = MIN(es->cursor_info.cursor_offset + increment, get_tid_text_size(es->main_buffer_tid));
	}
	else if (es->mode == EDITOR_MODE_HEX) {
		s64 lines_between_cursor_and_endline = es->cursor_info.last_line - es->cursor_info.cursor_line;
		s64 max_count = MAX(es->cursor_info.this_line_count, es->cursor_info.previous_line_count);
		s64 remaining = max_count - es->cursor_info.cursor_column;
		s64 num_lines = MAX((s64)ceilf((float)increment / max_count) - lines_between_cursor_and_endline, 0);
		if (num_lines > 0 &&
			increment % max_count > 0 &&
			increment % max_count < remaining) {
			num_lines--;
		}

		if (num_lines == 0) {
			// the increment is less than 1 line
			if (es->cursor_info.last_line == es->cursor_info.cursor_line &&
				es->cursor_info.cursor_column == es->cursor_info.this_line_count &&
				get_spare_lines(es) == 0) {
				if (es->cursor_info.cursor_offset + increment < get_tid_text_size(es->main_buffer_tid)) {
					scroll_down_ascii(es, es->cursor_info.this_line_count + 1);
				}
			}
			es->cursor_info.cursor_offset += increment;
		} else if (num_lines > 0) {
			s32 spare_lines = get_spare_lines(es);

			s64 nlines = num_lines - spare_lines;
			//s64 num_to_skip = (es->cursor_info.this_line_count - es->cursor_info.cursor_column) + (max_count - es->cursor_info.cursor_column) + (max_count * nlines);
			s64 num_to_skip = max_count * nlines;
			
			if(nlines > 0) scroll_down_ascii(es, num_to_skip);
			es->cursor_info.cursor_offset += increment;
		}
	}
	
}

void cursor_down(Editor_State* es, s64 incr)
{
	if (es->mode == EDITOR_MODE_HEX) {
		s64 count_to_skip = MIN(es->cursor_info.this_line_count, get_tid_text_size(es->main_buffer_tid) - es->cursor_info.cursor_offset);
		if (CURSOR_RELATIVE_OFFSET + count_to_skip <= get_tid_text_size(es->main_buffer_tid) && es->cursor_info.next_line_count > 0) {
			es->cursor_info.cursor_offset += count_to_skip;
		} else {
			//if(es->cursor_info.next_line_count)
			if (es->cursor_info.cursor_offset + count_to_skip == get_tid_text_size(es->main_buffer_tid)) {
				return;
			}
			es->cursor_info.cursor_offset += count_to_skip;
			scroll_down_ascii(es, es->cursor_info.this_line_count);
		}
	}
	else if (es->mode == EDITOR_MODE_ASCII) {
		if (es->line_wrap) {
			// todo
		}
		else {
			s64 count_from_cursor_to_next_lf = get_cursor_info(es->main_buffer_tid, es->cursor_info.cursor_offset, false, true, false).next_line_break.lf;
			if (count_from_cursor_to_next_lf == -1) {
				// if we are at the last line in the text
				return;
			}
			count_from_cursor_to_next_lf -= es->cursor_info.cursor_offset;

			s64 count_of_next_line = get_cursor_info(es->main_buffer_tid, es->cursor_info.cursor_offset + count_from_cursor_to_next_lf + 1, false, true, false).next_line_break.lf;
			if (count_of_next_line == -1) {
				// if we are at the penultima line we won't have a \n at the end of the text
				count_of_next_line = get_tid_text_size(es->main_buffer_tid) - (es->cursor_info.cursor_offset + count_from_cursor_to_next_lf + 1);
			} else {
				// otherwise proceed normally
				count_of_next_line -= es->cursor_info.cursor_offset + count_from_cursor_to_next_lf + 1;
			}

			s64 cursor_column = es->cursor_info.cursor_column;
			s64 snap = es->cursor_info.cursor_snaped_column;

			s64 count_to_skip = MIN(MAX(cursor_column, snap) + count_from_cursor_to_next_lf + 1, count_from_cursor_to_next_lf + 1 + count_of_next_line);
			if (count_to_skip < 0) return;

			if (CURSOR_RELATIVE_OFFSET + count_to_skip <= get_tid_text_size(es->main_buffer_tid) && es->cursor_info.next_line_count > 0) {
				// case in which we are inside the area of rendering
				es->cursor_info.cursor_offset += count_to_skip;
			} else {
				// the next line is outside the view of the window
				es->cursor_info.cursor_offset += count_to_skip;
				scroll_down_ascii(es, es->first_line_count);
			}
		}
	}
}

void cursor_up(Editor_State* es, s64 incr)
{
	if (es->mode == EDITOR_MODE_HEX) {
		if (CURSOR_RELATIVE_OFFSET - es->cursor_info.cursor_column - 1 < 0) {
			if (es->cursor_info.block_offset == 0) return;
			//if (es->cursor_info.cursor_offset - es->cursor_info.this_line_count < 0) return;
			assert(es->cursor_info.cursor_line == 0);

			s64 skip_amount = es->cursor_info.this_line_count;
			if (es->cursor_info.block_offset - skip_amount < 0) {
				skip_amount = es->cursor_info.block_offset;
			}
			scroll_up_ascii(es, skip_amount);
			es->cursor_info.cursor_offset = MAX(0, es->cursor_info.cursor_offset - es->cursor_info.this_line_count);
		}
		else {
			es->cursor_info.cursor_offset = MAX(0, es->cursor_info.cursor_offset - es->cursor_info.previous_line_count);
		}
	}
	else if (es->mode == EDITOR_MODE_ASCII) {
		cursor_info cinfo;
		int snap = MAX(0, es->cursor_info.cursor_snaped_column);
		s64 back_amt = es->cursor_info.cursor_offset - es->cursor_info.cursor_column - 1;

		if (back_amt < 0) return;	// this is the first line in the text, no need to go up

		cinfo = get_cursor_info(es->main_buffer_tid, es->cursor_info.cursor_offset - es->cursor_info.cursor_column - 1, false, true, false);
		s64 previous_line_count = (es->cursor_info.cursor_offset - es->cursor_info.cursor_column - 1) - cinfo.previous_line_break.lf;

		int c = previous_line_count - MAX(snap, es->cursor_info.cursor_column - 1);
		if (c <= 0) c = 1;
		c += es->cursor_info.cursor_column;

		// if the cursor is on the first line
		if (CURSOR_RELATIVE_OFFSET - es->cursor_info.cursor_column - 1 < 0) {
			assert(es->cursor_info.cursor_line == 0);
			scroll_up_ascii(es, previous_line_count);
		}

		es->cursor_info.cursor_offset -= c;
	}
}

void cursor_home(Editor_State* es, s64 incr)
{
	es->cursor_info.cursor_offset -= es->cursor_info.cursor_column;
	es->cursor_info.cursor_snaped_column = 0;
}

void cursor_end(Editor_State* es, s64 incr)
{
	s64 value = es->cursor_info.this_line_count - es->cursor_info.cursor_column;
	s64 is_final = 0;
	if (es->cursor_info.cursor_offset + value < get_tid_text_size(es->main_buffer_tid)) {
		value--;
		is_final++;
	}
	es->cursor_info.cursor_offset += value;
	es->cursor_info.cursor_snaped_column = es->cursor_info.this_line_count + is_final;
}

void cursor_page_down(Editor_State* es, s64 incr) {
	print("PAGE DOWN PRESSED\n");
	s64 lines_between_cursor_and_endline = es->cursor_info.last_line - es->cursor_info.cursor_line;

	if (es->mode == EDITOR_MODE_HEX) {
		s64 remaining_bytes = get_tid_text_size(es->main_buffer_tid) - es->cursor_info.cursor_offset;
		s64 max_count = MAX(MAX(es->cursor_info.this_line_count, es->cursor_info.next_line_count), es->cursor_info.previous_line_count);
		s64 lines_until_end = remaining_bytes / max_count;
		s64 num_to_skip = MIN(lines_between_cursor_and_endline, lines_until_end) * max_count;
		s32 spare_lines = get_spare_lines(es);
		if(spare_lines <= 0) scroll_down_ascii(es, num_to_skip);
		es->cursor_info.cursor_offset += num_to_skip;
	}
	
}

void cursor_page_up(Editor_State* es, s64 incr) {
	print("PAGE UP PRESSED\n");
}

void cursor_change_by_click(Editor_State* es, int x, int y)
{
	float xf = (float)x;
	float yf = (float)y;

	yf = win_state.win_height - yf;

	es->cursor_info.handle_seek = true;
	es->cursor_info.seek_position.x = xf;
	es->cursor_info.seek_position.y = yf;
}

void editor_select_all(Editor_State* es)
{
	// @TODO
}

void editor_force_selection(Editor_State* es, s64 cursor_begin, s64 cursor_end)
{
	es->selecting = true;
	cursor_force(es, cursor_begin);
	es->cursor_info.selection_offset = cursor_end;
}