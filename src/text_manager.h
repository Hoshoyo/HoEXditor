#ifndef HOHEX_TEXT_MANAGER_H
#define HOHEX_TEXT_MANAGER_H

#include "common.h"
#include "text.h"

extern u64 _tm_text_size;
extern u64 _tm_valid_bytes;

#define BLOCK_FILL_RATIO 0.5

// API initialization
s32 load_file(u8* filename);
s32 end_text_api();

// Returns a text buffer.
u8* get_text_buffer(u64 size, u64 cursor_begin);
s32 set_cursor_begin(u64 cursor_begin);
s32 insert_text(u8* text, u64 size, u64 cursor_begin);
// delete_text: u8* text is optional. If not null, the deleted text will be copied. text must already be allocated.
s32 delete_text(u8* text, u64 size, u64 cursor_begin);
s32 refresh_buffer();

u8* get_text_as_contiguous_memory(u64* text_size);

void check_text();
void check_arenas();

// internal
internal s32 fill_buffer();
internal ho_block* get_initial_block_at_cursor(u32* block_position, u64 cursor_begin);
internal s32 move_block_data(ho_block* block, u32 initial_block_position, u64 size, u8* memory_position);
internal void fill_blocks_with_text(u8* data, s64 data_size, u32 block_fill_value);

#endif
