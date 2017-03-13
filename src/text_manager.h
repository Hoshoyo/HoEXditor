#ifndef HOHEX_TEXT_MANAGER_H
#define HOHEX_TEXT_MANAGER_H

#include "common.h"
#include "text.h"

extern u64 _tm_text_size[MAX_FILES_OPEN];
extern u64 _tm_valid_bytes[MAX_FILES_OPEN];

#define BLOCK_FILL_RATIO 0.5

// API initialization
// load_file: load a new file. receives filename. returns id and error status.
s32 load_file(s32* id, u8* filename);
// finalize_file: finalize file, receives id. returns error status.
s32 finalize_file(s32 id);

// Returns a text buffer.
u8* get_text_buffer(s32 id, u64 size, u64 cursor_begin);
s32 set_cursor_begin(s32 id, u64 cursor_begin);
s32 insert_text(s32 id, u8* text, u64 size, u64 cursor_begin);
// delete_text: u8* text is optional. If not null, the deleted text will be copied. text must already be allocated.
s32 delete_text(s32 id, u8* text, u64 size, u64 cursor_begin);
s32 refresh_buffer(s32 id);
ho_block* get_initial_block_at_cursor(s32 id, u32* block_position, u64 cursor_begin);

// functions that move block to contiguous text
u8* get_text_as_contiguous_memory(s32 id, u64* text_size);
s32 move_block_data(ho_block* block, u32 initial_block_position, u64 size, u8* memory_position);

void check_text(s32 id);
void check_arenas(s32 id);

// internal
internal s32 configure_text_events(s32 id);
internal void store_file_name(s32 id, u8* filename);
internal s32 fill_buffer(s32 id);
internal void fill_blocks_with_text(s32 id, u8* data, s64 data_size, u32 block_fill_value);

#endif
