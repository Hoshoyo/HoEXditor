#include "common.h"

#ifndef HOHEX_TEXT_H
#define HOHEX_TEXT_H

#define BLOCK_SIZE 2048         // 2 KB
#define ARENA_SIZE 1048576      // 1 MB
#define BLOCKS_PER_ARENA 512    // must be multiply of 8
#define BLOCKS_PER_CONTAINER 8

typedef struct ho_block_struct
{
  u8* data;
  u32 total_size;
  u32 occupied;
  u32 empty;
} ho_block;

typedef struct ho_block_container_struct
{
  ho_block blocks[BLOCKS_PER_CONTAINER];
  struct ho_block_container_struct* next;
} ho_block_container;

typedef struct ho_text_struct
{
  u32 num_blocks;
  ho_block_container* block_container;
} ho_text;

typedef struct ho_deleted_block_struct
{
  u32 block_number;
  struct ho_deleted_block_struct* next;
} ho_deleted_block;

typedef struct ho_arena_descriptor_struct
{
  ho_deleted_block* first_deleted_block;
  ho_deleted_block* last_deleted_block;
  u8 block_status_bitmap[BLOCKS_PER_ARENA/8];
  void* initial_address;
  u32 id;
  struct ho_arena_descriptor_struct* next;
} ho_arena_descriptor;

typedef struct ho_arena_manager_struct
{
  u32 num_arenas;
  ho_arena_descriptor* arena;
} ho_arena_manager;

u32 init_text();
void print_block(ho_block block);
void print_arena_descriptor(ho_arena_descriptor arena_descriptor);
void print_arena_manager(ho_arena_manager arena_manager);
void print_text(ho_text text);

// Create a new block in front of the existing block. The existing block is splitted.
// The position of the existing block must be correct, and must be calculated outside the function. The absolute position begins in 0.
// Function returns the new block, which position is existing_block_position + 1.
ho_block* append_block(ho_block* existing_block, u32 existing_block_absolute_position);

// aux functions
ho_block* put_new_block_and_move_others_to_right(ho_block new_block, u32 array_position, ho_block_container* block_container, u32 absolute_position);
void* fill_arena_bitmap_and_return_address(ho_arena_descriptor* arena_descriptor);

u8* request_new_block_data();

#endif
