#include "common.h"

#ifndef HOHEX_TEXT_H
#define HOHEX_TEXT_H

#define BLOCK_SIZE 2048         // 2 KB
#define ARENA_SIZE 1048576      // 1 MB
#define BLOCKS_PER_ARENA 512
#define BLOCKS_PER_CONTAINER 512

typedef struct ho_block_struct
{
  u8* data;
  u32 total_size;
  u32 occupied;
  u32 empty;
} ho_block;

typedef struct ho_block_container_struct
{
  ho_block* blocks[BLOCKS_PER_CONTAINER];
  struct ho_block_container_struct* next;
} ho_block_container;

typedef struct ho_text_struct
{
  u32 num_blocks;
  ho_block_container* block_container;
} ho_text;

typedef struct ho_arena_descriptor_struct
{
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

int init_text();
void print_block(ho_block block);
void print_arena_descriptor(ho_arena_descriptor arena_descriptor);

#endif
