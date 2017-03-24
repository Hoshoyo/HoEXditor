#ifndef HOHEX_TEXT_MANAGER_H
#define HOHEX_TEXT_MANAGER_H

#include "common.h"
#include "text.h"

#define BLOCK_FILL_RATIO 0.5

typedef struct cursor_info_struct cursor_info;
typedef struct line_breaks_types_struct line_break_types;

struct line_breaks_types_struct
{
  s64 cr;
  s64 lf;
  s64 crlf;
};

struct cursor_info_struct
{
  line_break_types line_number;
  line_break_types previous_line_break;
  line_break_types next_line_break;
};

// API initialization
s32 create_tid(text_id* tid, bool is_block_text);
// finalize_tid: finalize text_id, receives tid. returns error status.
s32 finalize_tid(text_id tid);

// load_file: load a new file. receives fila_path and text_id and error status.
// IMPORTANT: IT MUST BE AN EMPTY TID, OR MEMORY LEAKS MAY BE GENERATED.
s32 load_file(text_id tid, u8* file_path);
// create_real_buffer: only for contiguous text. Must be created once per tid.
s32 create_real_buffer(text_id tid, u64 size);

// Returns a text buffer.
u8* get_text_buffer(text_id tid, u64 size, u64 cursor_begin);
u64 get_tid_valid_bytes(text_id tid);
u64 get_tid_text_size(text_id tid);
u8* get_tid_file_name(text_id tid);
s32 set_cursor_begin(text_id tid, u64 cursor_begin);
s32 insert_text(text_id tid, u8* text, u64 size, u64 cursor_begin);
// delete_text: u8* text is optional. If not null, the deleted text will be copied. text must already be allocated.
s32 delete_text(text_id tid, u8* text, u64 size, u64 cursor_begin);
s32 refresh_buffer(text_id tid);
s32 change_file_path(text_id tid, u8* file_path);
ho_block* get_initial_block_at_cursor(text_id tid, u32* block_position, u64 cursor_begin);
cursor_info get_cursor_info(text_id tid, u64 cursor_position);
bool test_if_pattern_match(ho_block* block, u32 block_position, u8* pattern, u64 pattern_length);
bool test_if_pattern_match_backwards(ho_block* block, u32 block_position, u8* pattern, u64 pattern_length);

// functions that move block to contiguous text
u8* get_text_as_contiguous_memory(text_id tid, u64* text_size);
s32 copy_text_to_contiguous_memory_area(text_id tid, u64 cursor_begin, u64 size, u8* memory_ptr);
s32 move_block_data(ho_block* block, u32 initial_block_position, u64 size, u8* memory_position);

void check_text(text_id tid);
void check_arenas(text_id tid);

// internal
internal s64 find_next_pattern_backwards(text_id tid, u64 cursor_begin, u64 cursor_end, u8* pattern, u64 pattern_length);
internal s64 find_next_pattern_forward(text_id tid, u64 cursor_begin, u64 cursor_end, u8* pattern, u64 pattern_length);
internal s64 get_number_of_pattern_occurrences(text_id tid, u64 cursor_begin, u64 cursor_end, u8* pattern, u64 pattern_length, bool skip_first_position);
internal s32 configure_text_events(text_id tid);
internal void refresh_cursor_info_reference(text_id tid);
internal u64 get_cursor_line_number(s32 id, u64 cursor_position);
internal s32 fill_buffer(text_id tid);
internal void fill_blocks_with_text(text_id tid, u8* data, s64 data_size, u32 block_fill_value);

#endif
