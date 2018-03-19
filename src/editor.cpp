
typedef struct {
	s64 abs_offset;
	s64 rel_offset;

	s64 line;
	s64 column;

	s64 abs_line;
} Cursor;

typedef struct {
	bool initialized;

	Cursor cursor;

	// buffer
	s8 raw[180 * 8 + 5] = { 0 };
	s8* at;
	s64 buffer_at_offset;
	s64 buffer_length;
	s64 buffer_characters_left;
	s64 abs_line;

	// view
	s32 lines;				// actual number of lines displayed at the moment
	s32 columns;			// actual max number of columns displayed at the moment
	s32 max_lines;			// maximum amount of lines allowed
	s32 max_columns;		// maximum amount of columns allowed
	r32 char_max_width;		// maximum width of a character
	r32 left_margin;
	r32 right_margin;
	r32 top_margin;		// TODO(psv): not yet used
	r32 bot_margin; 	// TODO(psv): not yet used
	r32 pixel_spacing;		// the spacing between each byte
	bool render_lines;
	bool render_lines_hex;

	RectBase base;
} Editor;

static Editor editor = {0};

void cursor_up(Editor* e, s32 num_lines);
void cursor_down(Editor* e, s32 num_lines);
void cursor_left(Editor* e, s32 num_cells);
void cursor_right(Editor* e, s32 num_cells);

internal void editor_initialize() {
	for (u32 i = 0; i < ARRAY_COUNT(editor.raw); ++i)
		editor.raw[i] = i;

	editor.initialized            = true;

	editor.abs_line               = 0;
	editor.lines                  = 0;
	editor.columns                = 0;
	editor.max_lines              = 50;
	editor.max_columns            = 16;
	editor.at                     = editor.raw;
	editor.buffer_at_offset       = 0;
	editor.buffer_length          = ARRAY_COUNT(editor.raw) - 1;
	editor.buffer_characters_left = editor.buffer_length;
	editor.char_max_width         = font_info.max_width + 1.0f;
	editor.left_margin            = 2.0f;
	editor.right_margin           = 2.0f;
	editor.top_margin             = 2.0f;
	editor.bot_margin             = 2.0f;
	editor.pixel_spacing          = 4.0f;
	
	editor.render_lines           = true;
	editor.render_lines_hex       = false;

	editor.base = {0.0f, 100.0f, 100.0f, 28.0f};
	
	editor.cursor.abs_offset      = 0;
	editor.cursor.rel_offset      = 0;
	editor.cursor.line            = 0;
	editor.cursor.abs_line        = 0;
	editor.cursor.column          = 0;
}

internal void editor_display_info() {
	// cursor info
	s8 buffer[512] = {0};
	s32 len = sprintf(buffer, "cursor(line): %lld, (column): %lld", editor.cursor.line, editor.cursor.column);
	string text = {buffer, len};
	hm::vec2 position = {0.0f, 5.0f};
	hm::vec4 color = {1, 0.3f, 0.3f, 1};
	render_text(&font_info, text, position, color);

	memset(buffer, 0, 512);
	len = sprintf(buffer, "cursor(abs_offset): %lld, (rel_offset): %lld, editor(abs_line): %lld, editor(lines): %d", editor.cursor.abs_offset, editor.cursor.rel_offset, editor.abs_line, editor.lines);
	text.length = len;
	position = {0.0f, 5.0f + font_info.max_height};
	render_text(&font_info, text, position, color);

	font_rendering_flush(&font_info, font_info.shader);
}

internal void editor_update_cursor(Editor* e, s32 new_columns) {
	s64 absolute_offset = e->cursor.abs_offset;
	e->cursor.rel_offset = 0;
	e->cursor.abs_line = 0;
	e->cursor.abs_offset = 0;
	e->cursor.line = 0;
	e->cursor.column = 0;
	e->columns = new_columns;
	e->buffer_at_offset = 0;
	e->at = e->raw;
	e->abs_line = 0;
	cursor_right(e, absolute_offset);

	int x = 0;
	//s64 relative_offset = e->cursor.rel_offset;
	//e->cursor.rel_offset = 0;
	//e->cursor.abs_line -= e->cursor.line;
	//e->cursor.abs_offset -= relative_offset;
	//e->cursor.line = 0;
	//e->cursor.column = 0;
	//e->columns = new_columns;
	//cursor_right(e, relative_offset);
}

internal void editor_update_and_render() 
{
	if (!editor.initialized)
		return;

	RectBase base = editor.base;
	s32 width  = win_state.win_width - base.left;
	s32 height = win_state.win_height - base.bottom;

	hm::vec4 vs_blue_color = hm::vec4(0.0f, 122.0f / 255.0f, 204.0f / 255.0f, 1.0f);

	s32 max_lines              = editor.max_lines;
	s32 max_columns            = editor.max_columns;
	s8* at                     = editor.at;
	s64 buffer_at_offset       = editor.buffer_at_offset;
	s64 buffer_length          = editor.buffer_length;
	s64 buffer_characters_left = editor.buffer_characters_left;
	
	r32 char_max_width = editor.char_max_width;
	r32 left_margin    = editor.left_margin;
	r32 right_margin   = editor.right_margin;
	r32 top_margin     = editor.top_margin;
	r32 bot_margin     = editor.bot_margin;
	r32 pixel_spacing  = editor.pixel_spacing;

	r32 baseleft    = base.left;
	r32 basebottom  = base.bottom;


	// Render lines
	r32 line_space = 0.0f;
	if(editor.render_lines) {
		// get space in the left handside
		line_space = 8.0f * char_max_width;
		baseleft += line_space;
	}

	r32 w           = baseleft + left_margin;
	r32 h           = basebottom + (r32)height;
	s32 count       = 0;
	s32 lines       = 0;
	s32 cols        = 0;
	s32 num_columns = 0;

	// Render Body
	for(; buffer_at_offset + count <= buffer_length; ++at, ++count) 
	{
		// Form the 2 nibbles to be rendered
		s8 tbuf[3] = { NUM_TO_HIGNIBBLE(at[0]), NUM_TO_LOWNIBBLE(at[0]), 0 };
		string text = { tbuf, 2 };

		// render at correct position
		hm::vec2 position = {left_margin + w, -(r32)font_info.max_height + h };

		render_text(&font_info, text, position, vs_blue_color);
		w += char_max_width * 2.0f + pixel_spacing;
		cols += 1;

		if(max_columns) {
			// if max_columns is specified, then render 
			// only that amount of columns
			if(cols == max_columns) {
				cols = 0;
				w = baseleft + left_margin;
				h -= font_info.max_height;
				lines += 1;
				num_columns = max_columns;
			}
		}
		// if max_columns is not specified (i.e. 0), then render
		// until there is no more space in the width
		if (w + char_max_width * 2.0f + right_margin > (r32)width) {
			w = baseleft + left_margin;
			h -= font_info.max_height;
			lines += 1;
			num_columns = cols;
			cols = 0;
		}

		if(max_lines) {
			// if max_lines is specified, then render
			// only that amount of lines
			if(lines == max_lines) {
				break;
			}
		} else {
			// if max_lines is not specified (i.e. 0), then render
			// until there is no more space for lines
		}
		if(h - font_info.max_height - bot_margin < basebottom) {
			break;
		}
	}
	// TODO(psv): recalculate cursor line and column
	s32 rendered_columns = MAX(cols, num_columns) * 2;
	if(editor.columns != rendered_columns) {
		editor_update_cursor(&editor, rendered_columns);
	}

	//editor.columns = rendered_columns;
	s32 extra_line = (cols < num_columns && cols != 0) ? 1 : 0;
	editor.lines   = lines + extra_line;	// acount for last line incomplete

	// Render lines
	if (editor.render_lines) {
		s32 count = 0;
		r32 h = basebottom + (r32)height;
		for (; count < max_lines && count <= editor.lines; ++count) {
			hm::vec4 line_numbers_color = hm::vec4(0.7f, 0.7f, 0.8f, 1.0f);
			s8 buffer[64] = { 0 };
			u64 baseoffset = (u32)(editor.buffer_at_offset + (editor.columns * count) / 2);
			s32 len = 0;
			if(editor.render_lines_hex)
				len = u32_to_str_base16(baseoffset, true, buffer);
			else
				len = u32_to_str_base10(baseoffset, buffer); // len = u32_to_str_base10(count, buffer);
			string s = { buffer, len };
			hm::vec2 position = { left_margin + baseleft - line_space, -(r32)font_info.max_height + h };
			render_text(&font_info, s, position, line_numbers_color);

			h -= font_info.max_height;

			if (max_lines) {
				// if max_lines is specified, then render
				// only that amount of lines
				if (count == max_lines) {
					break;
				}
			}
			else {
				// if max_lines is not specified (i.e. 0), then render
				// until there is no more space for lines
			}
			if (h - font_info.max_height - bot_margin < basebottom) {
				break;
			}

		}
	}

	font_rendering_flush(&font_info, font_info.shader);

	{
		// Render cursor

		// size of a jump from 1 byte to the other horizontally
		r32 horiz_jump = (char_max_width * 2.0f) + pixel_spacing;
		r32 l = baseleft + left_margin + (editor.cursor.column / 2) * horiz_jump;
		// if the cursor is in an odd position, it is in the middle of the byte,
		// therefore should add the amount to get in between both characters
		if(editor.cursor.column % 2 != 0) {
			l += char_max_width;
		}
		r32 r = l + 1.0f;
		r32 t = basebottom + height - (font_info.max_height) * editor.cursor.line - 1.0f;
		r32 b = t - font_info.max_height - 2.0f;
		immediate_quad(l, r, t, b, hm::vec4(1,1,1,1.0f));
	}
	editor_display_info();
	immediate_rect_border(&base, 1, 0.0f, width, height, 0.0f, hm::vec4(1, 1, 0, 1));
}

s64 editor_last_line(Editor* e) {
	assert(e->columns % 2 == 0);
	s64 bytes_per_column = e->columns / 2;
	s64 lines = e->buffer_length / bytes_per_column;
	if (e->buffer_length % bytes_per_column > 0)
		lines++;
	return lines - 1;	// index of line instead of count
}
s64 editor_last_relative_line(Editor* e) {
	return e->lines - 1;
}

s64 editor_cells_until_eof(Editor* e) {
	return e->buffer_length * 2 - e->cursor.abs_offset + 2;
}

void cursor_up(Editor* e, s32 num_lines){
	// if the cursor is at the top, no point changing it
	if(e->cursor.abs_line == 0) return;
	
	num_lines = MIN(e->cursor.abs_line, num_lines);

	for (s32 n = 0; n < num_lines; ++n) {
		if (e->cursor.line > 0) {
			// the cursor is on the middle or the end of the screen
			// and the destination line is within the screen
			e->abs_line -= 1;
			e->cursor.line -= 1;
			e->cursor.abs_line -= 1;
			e->cursor.abs_offset -= e->columns;
			e->cursor.rel_offset -= e->columns;
		}
		else {
			// the cursor is on the top or near the top,
			// but there is file to be read
			s32 delta = (e->columns) / 2;
			assert((e->columns) % 2 == 0);
			e->buffer_at_offset -= delta;
			e->at -= delta;
			e->abs_line -= 1;
			e->cursor.abs_line -= 1;
			e->cursor.abs_offset -= delta * 2;
		}
	}
}

void cursor_down(Editor* e, s32 num_lines) {
	for(s32 n = 0; n < num_lines; ++n){
		// going beyond the lines at display
		s64 cells_until_next_line = e->columns - e->cursor.column;
		s64 next_line_offset = e->cursor.abs_offset + cells_until_next_line;
		// the number of cells in the next line is either what is left in the buffer
		// or the maximum number of columns
		s32 cells_at_next_line = MIN((e->buffer_length * 2) + 2 - next_line_offset, e->columns);
		if (cells_at_next_line <= 0)
			return;
		s32 delta = MIN(cells_at_next_line, e->cursor.column);

		if(e->cursor.line + 1 >= e->lines) {
			s32 delta = MIN(cells_at_next_line, e->cursor.column);
			e->buffer_at_offset += e->columns / 2;
			e->at += e->columns / 2;
			e->cursor.abs_line += 1;
			e->abs_line += 1;
			e->cursor.abs_offset += cells_until_next_line + delta;
			e->cursor.column = delta;
		} else {
			e->cursor.line += 1;
			e->cursor.abs_line += 1;
			e->abs_line += 1;
			e->cursor.abs_offset += cells_until_next_line + delta;//e->columns;
			e->cursor.rel_offset += cells_until_next_line + delta;//e->columns;
			e->cursor.column = delta;
		}
	}
}

void cursor_right(Editor* e, s32 num_cells){
	if (num_cells <= 0) {
		return;
	}
	s64 num_cells_left = (e->buffer_length * 2 + 2) - e->cursor.abs_offset;
	s64 num_cells_moving = MIN(num_cells, num_cells_left);

	s64 max_columns = e->columns;

	s64 lines_down = num_cells_moving / max_columns;
	s64 add_column = num_cells_moving % max_columns;

	printf("lines %lld, column %lld\n", lines_down, add_column);

	cursor_down(e, lines_down);

	e->cursor.column += add_column;
	e->cursor.abs_offset += add_column;
	e->cursor.rel_offset += add_column;

	/*
	if (num_cells == 0) return;
	if (e->cursor.abs_offset + num_cells > (e->buffer_length + 1) * 2) {
		num_cells = e->buffer_length * 2 - e->cursor.abs_offset + 2;
	}

	if (e->cursor.column + num_cells + 1 > e->columns) {
		s32 to_move = num_cells;
		to_move -= (e->columns - e->cursor.column - 1);
		cursor_right(e, (e->columns - e->cursor.column - 1));
		s32 lines_down = to_move / e->columns + 1;
		to_move = to_move % e->columns;
		cursor_down(e, lines_down);

		s64 last_rel_line = e->lines - 1;
		s64 current_line = e->cursor.line;

		if (last_rel_line == current_line) {
			//printf("to move: %d\n", to_move);
			cursor_right(e, to_move - 1);
		} else {
			cursor_left(e, e->columns - to_move);
		}
	} else {
		e->cursor.column += num_cells;
		e->cursor.abs_offset += num_cells;
		e->cursor.rel_offset += num_cells;
	}
	*/
}

void cursor_left(Editor* e, s32 num_cells) {
	if (num_cells > e->cursor.abs_offset) {
		num_cells = e->cursor.abs_offset;
	}

	if (num_cells > e->cursor.column) {
		s32 to_move = num_cells;
		to_move -= e->cursor.column;
		cursor_left(e, e->cursor.column);
		s32 lines_up = to_move / e->columns + 1;
		to_move = to_move % e->columns;
		cursor_up(e, lines_up);
		cursor_right(e, e->columns - to_move);
	} else {
		e->cursor.column -= num_cells;
		e->cursor.abs_offset -= num_cells;
		e->cursor.rel_offset -= num_cells;
	}
}