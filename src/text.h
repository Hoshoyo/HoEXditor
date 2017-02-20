#ifndef HOHEX_TEXT_H
#include "common.h"

#define HOHEX_TEXT_H

#define BLOCK_SIZE 8         // 2 KB
#define ARENA_SIZE 1048576      // 1 MB
#define BLOCKS_PER_ARENA 8    // must be multiply of 8
#define BLOCKS_PER_CONTAINER 8

typedef struct ho_block_data_struct ho_block_data;
typedef struct ho_block_struct ho_block;
typedef struct ho_block_container_struct ho_block_container;
typedef struct ho_text_struct ho_text;
typedef struct ho_deleted_block_struct ho_deleted_block;
typedef struct ho_arena_descriptor_struct ho_arena_descriptor;
typedef struct ho_arena_manager_struct ho_arena_manager;

extern ho_text main_text;
extern ho_arena_manager main_arena_manager;

// BLOCKS

struct ho_block_data_struct
{
  u8* data;
  ho_arena_descriptor* arena;
};

struct ho_block_struct
{
  ho_block_data block_data;
  u32 total_size;
  u32 occupied;
  u32 empty;
  u32 position_in_container;  // begins at 0
  ho_block_container* container;
};

struct ho_block_container_struct
{
  ho_block blocks[BLOCKS_PER_CONTAINER];
  u32 num_blocks_in_container;
  u32 total_occupied;
  ho_block_container* next;
};

struct ho_text_struct
{
  u32 num_blocks;
  ho_block_container* block_container;
};

// ARENA

struct ho_deleted_block_struct
{
  u32 block_number; // begins at 0
  ho_deleted_block* next;
};

struct ho_arena_descriptor_struct
{
  ho_deleted_block* first_deleted_block;
  ho_deleted_block* last_deleted_block;
  u8 block_status_bitmap[BLOCKS_PER_ARENA/8];
  void* initial_address;
  u32 id;
  ho_arena_descriptor* next;
  ho_arena_descriptor* previous;
};

struct ho_arena_manager_struct
{
  u32 num_arenas;
  ho_arena_descriptor* arena;
};

// external API
u32 init_text();
u32 destroy_text();
// Create a new block in front of the existing block.
// Function returns a pointer the new block, which will already be inside its own block_container.
ho_block* append_block(ho_block existing_block);
// Insert text inside 'block', beginning in data_position. The first position is 0. Each byte is a position.
// If split_if_necessary is true, the block will be splitted in case of not having enough space.
// If split_if_necessary is false and more space is needed, function will not perform its task and return error.
// data_position must be a valid number - it must be lower than block's data size.
u32 insert_text_in_block(ho_block* block, u8* text, u32 data_position, u32 text_size, bool split_if_necessary);
// Delete text inside 'block', beginning in data_position. The first position is 0. Each byte is a position.
// If recursive_if_necessary is true, the removal may reach other blocks.
// If recursive_if_necessary is false, function may not perform its task and return error.
// data_position must be a valid number - it must be lower than block's data size.
u32 delete_text_in_block(ho_block* block, u32 data_position, u32 text_size, bool recursive_if_necessary);
// Delete a block
void delete_block(ho_block block_to_be_deleted);

// aux functions
internal ho_block* put_new_block_and_move_others_to_right(ho_block new_block, ho_block existing_block);
internal void delete_block_and_move_others_to_left(ho_block block_to_be_deleted);
internal void* fill_arena_bitmap_and_return_address(ho_arena_descriptor* arena_descriptor);
internal ho_block_data request_new_block_data();
internal void free_block_data(ho_block_data block_data);
internal ho_arena_descriptor* create_new_arena(ho_arena_descriptor* last_arena);
internal bool is_arena_empty(ho_arena_descriptor* arena);
internal void free_arena(ho_arena_descriptor* arena);
// Splits first block. Half of the block data will stay in the original block. The other half will be moved to new block.
// new_block must be a brand new_block or an empty block. If not, data may be overwritten.
internal void split_block(ho_block* block_to_be_split, ho_block* new_block);

// print functions
void print_block(ho_block block);
void print_arena_descriptor(ho_arena_descriptor arena_descriptor);
void print_arena_manager(ho_arena_manager arena_manager);
void print_text(ho_text text);

#endif
