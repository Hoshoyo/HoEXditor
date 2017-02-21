#ifndef HOHEX_TEXT_MANAGER_H
#define HOHEX_TEXT_MANAGER_H

#include "common.h"
#include "text.h"

extern u64 _tm_text_size;

#define BLOCK_FILL_RATIO 0.5

// API initialization
u32 init_text_api(u8* filename);
u32 end_text_api();

// Returns a text buffer.
u8* get_text_buffer(u64 size, u64 cursor_begin);
u32 set_cursor_begin(u64 cursor_begin);
u32 insert_text(u8* text, u64 size, u64 cursor_begin);
u32 delete_text(u64 cursor_begin, u64 size);

// internal
internal u32 fill_buffer();
internal ho_block* get_initial_block_at_cursor_begin(u32* block_position);
internal u32 move_block_data(ho_block* block, u32 initial_block_position, u64 size, u8* memory_position);
internal void fill_blocks_with_text(u8* data, s64 data_size, u32 block_fill_value);

#endif
