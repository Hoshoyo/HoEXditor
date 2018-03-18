
typedef struct {
	s64 abs_offset;
	s64 rel_offset;

	s64 line;
	s64 column;

	s64 abs_line;
} Cursor;

typedef struct {
	Cursor cursor;

	// buffer
	s8 raw[10 * 8] = { 0 };
	s8* at;
	s64 buffer_at_offset;
	s64 buffer_length;
	s64 buffer_characters_left;

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
} Editor;

static Editor editor = {0};

internal void editor_initialize() {
	for (u32 i = 0; i < ARRAY_COUNT(editor.raw); ++i)
		editor.raw[i] = i;

	editor.lines                  = 0;
	editor.columns                = 0;
	editor.max_lines              = 10;
	editor.max_columns            = 8;
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
	len = sprintf(buffer, "cursor(abs_offset): %lld, (rel_offset): %lld", editor.cursor.abs_offset, editor.cursor.rel_offset);
	text.length = len;
	position = {0.0f, 5.0f + font_info.max_height};
	render_text(&font_info, text, position, color);

	font_rendering_flush(&font_info, font_info.shader);
}

internal void editor_update_and_render() 
{
	s32 width  = win_state.win_width;
	s32 height = win_state.win_height;

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

	r32 w     = left_margin;
	r32 h     = (r32)height;
	s32 count = 0;
	s32 lines = 0;
	s32 cols  = 0;
	s32 num_columns = 0;

	for(; count < buffer_length; ++at, ++count) 
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
				w = left_margin;
				h -= font_info.max_height;
				lines += 1;
				num_columns = max_columns;
			}
		} else {
			// if max_columns is not specified (i.e. 0), then render
			// until there is no more space in the width
			if (w + char_max_width * 2.0f + right_margin > (r32)width) {
				w = left_margin;
				h -= font_info.max_height;
				lines += 1;
				num_columns = cols;
			}
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
			if(h - font_info.max_height - bot_margin < 0.0f) {
				break;
			}
		}
	}
	editor.columns = MAX(cols, num_columns) * 2;
	editor.lines   = lines;

	font_rendering_flush(&font_info, font_info.shader);

	{
		// Render cursor

		// size of a jump from 1 byte to the other horizontally
		r32 horiz_jump = (char_max_width * 2.0f) + pixel_spacing;
		r32 l = left_margin + (editor.cursor.column / 2) * horiz_jump;
		// if the cursor is in an odd position, it is in the middle of the byte,
		// therefore should add the amount to get in between both characters
		if(editor.cursor.column % 2 != 0) {
			l += char_max_width;
		}
		r32 r = l + 1.0f;
		r32 t = height - (font_info.max_height) * editor.cursor.line - 1.0f;
		r32 b = t - font_info.max_height - 2.0f;
		immediate_quad(l, r, t, b, hm::vec4(1,1,1,1.0f));
	}
	editor_display_info();
}

void cursor_up(Editor* e, s32 num_lines);
void cursor_down(Editor* e, s32 num_lines);
void cursor_left(Editor* e, s32 num_cells);
void cursor_right(Editor* e, s32 num_cells);

void cursor_up(Editor* e, s32 num_lines){
	// if the cursor is at the top, no point changing it
	if(e->cursor.abs_line == 0) return;
	
	num_lines = MIN(e->cursor.abs_line, num_lines);

	if(e->cursor.line > num_lines - 1) {
		// the cursor is on the middle or the end of the screen
		// and the destination line is within the screen
		e->cursor.line -= num_lines;
		e->cursor.abs_line -= num_lines;
		e->cursor.abs_offset -= e->columns * num_lines;
		e->cursor.rel_offset -= e->columns * num_lines;
	} else {
		// the cursor is on the top or near the top,
		// but there is file to be read 
	}
}

void cursor_down(Editor* e, s32 num_lines) {
	s64 bytes_until_eof   = e->buffer_length - e->buffer_at_offset;
	s64 bytes_in_one_line = e->columns;

	// TODO(psv):
	// if the cursor is advancing more than what is left in the file
	// then don't move at all
	e->cursor.line += num_lines;
	e->cursor.abs_line += num_lines;
	e->cursor.abs_offset += e->columns * num_lines;
	e->cursor.rel_offset += e->columns * num_lines;
}

void cursor_right(Editor* e, s32 num_cells){
	if (e->cursor.abs_offset + num_cells > e->buffer_length * 2) {
		num_cells = e->buffer_length * 2 - e->cursor.abs_offset;
	}

	if (e->cursor.column + num_cells + 1 > e->columns) {
		s32 to_move = num_cells;
		to_move -= (e->columns - e->cursor.column - 1);
		cursor_right(e, (e->columns - e->cursor.column - 1));
		s32 lines_down = to_move / e->columns + 1;
		to_move = to_move % e->columns;
		cursor_down(e, lines_down);
		cursor_left(e, e->columns - to_move);
	} else {
		e->cursor.column += num_cells;
		e->cursor.abs_offset += num_cells;
		e->cursor.rel_offset += num_cells;
	}
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