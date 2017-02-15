#include "text.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>

ho_text main_text;
ho_arena_manager main_arena_manager;

int init_text()
{
  u32 aux;

  main_arena_manager.arena = malloc(sizeof(ho_arena_descriptor));
  if (main_arena_manager.arena == null)
    return -1;
  main_arena_manager.num_arenas = 1;
  main_arena_manager.arena->next = null;
  main_arena_manager.arena->initial_address = alloc(ARENA_SIZE, &(main_arena_manager.arena->id));

  for (aux=0; aux<BLOCKS_PER_ARENA/8; aux++)
    main_arena_manager.arena->block_status_bitmap[aux] = 0;

  main_text.block_container = malloc(sizeof(ho_block_container));
  main_text.block_container->next = null;
  main_text.block_container->blocks[0] = malloc(sizeof(ho_block));
  if (main_text.block_container->blocks[0] == null)
    return -1;
  main_text.block_container->blocks[0]->data = main_arena_manager.arena->initial_address;
  main_arena_manager.arena->block_status_bitmap[0] |= (1 << 0);
  main_text.block_container->blocks[0]->total_size = BLOCK_SIZE;
  main_text.block_container->blocks[0]->occupied = 0;
  main_text.block_container->blocks[0]->empty = BLOCK_SIZE;

  main_text.num_blocks = 1;


  // test
  main_text.block_container->blocks[0]->data = "IhaHigorII easily killed hoshoyo using fire bolt";
  main_text.block_container->blocks[0]->occupied += sizeof("IhaHigorII easily killed hoshoyo using fire bolt");
  main_text.block_container->blocks[0]->empty -= sizeof("IhaHigorII easily killed hoshoyo using fire bolt");
  print_block(*(main_text.block_container->blocks[0]));

  return 0;
}

void print_block(ho_block block)
{
  u32 aux;

  printf("Total Size: %u\n", block.total_size);
  printf("Occupied: %u\n", block.occupied);
  printf("Empty: %u\n", block.empty);
  printf("Data: ");

  for (aux=0; aux<block.occupied; aux++)
    printf("%c", *(block.data + aux));
}

void print_arena_descriptor(ho_arena_descriptor arena_descriptor)
{
  // to do
}
