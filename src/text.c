#include "text.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ho_text main_text;
ho_arena_manager main_arena_manager;

// to be improved
void copy_string(u8* dest, u8* src, u32 size)
{
  u32 aux;

  for (aux=0; aux<size; aux++)
    dest[aux] = src[aux];
}

// temporary function
void add_text_to_block(ho_block* block, u8* text)
{
  u32 sizeoftext = strlen(text);
  copy_string(block->data, text, sizeoftext);
  block->occupied += sizeoftext;
  block->empty -= sizeoftext;
}

u32 init_text()
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
  main_text.block_container->blocks[0].data = main_arena_manager.arena->initial_address;
  main_arena_manager.arena->block_status_bitmap[0] |= (1 << 0);
  main_text.block_container->blocks[0].total_size = BLOCK_SIZE;
  main_text.block_container->blocks[0].occupied = 0;
  main_text.block_container->blocks[0].empty = BLOCK_SIZE;
  add_text_to_block(&main_text.block_container->blocks[0], "Wow! This Is My First Block! How Nice!");

  main_text.num_blocks = 1;

  // CODE BELOW: ONLY TESTS ! [begin]
  /*copy_string(main_text.block_container->blocks[0].data, "IhaHigorII easily killed hoshoyo using fire bolt", sizeof("IhaHigorII easily killed hoshoyo using fire bolt"));
  main_text.block_container->blocks[0].occupied += sizeof("IhaHigorII easily killed hoshoyo using fire bolt");
  main_text.block_container->blocks[0].empty -= sizeof("IhaHigorII easily killed hoshoyo using fire bolt");

  main_text.num_blocks = 2;
  main_text.block_container->blocks[1].data = (((char*)main_arena_manager.arena->initial_address) + 1*BLOCK_SIZE);
  main_arena_manager.arena->block_status_bitmap[0] |= (1 << 1);
  main_text.block_container->blocks[1].total_size = BLOCK_SIZE;
  main_text.block_container->blocks[1].occupied = 0;
  main_text.block_container->blocks[1].empty = BLOCK_SIZE;

  copy_string(main_text.block_container->blocks[1].data, "Hoshoyo died while trying to run away from MDI.", sizeof("Hoshoyo died while trying to run away from MDI."));
  main_text.block_container->blocks[1].occupied += sizeof("Hoshoyo died while trying to run away from MDI.");
  main_text.block_container->blocks[1].empty -= sizeof("Hoshoyo died while trying to run away from MDI.");
*/
  // create dummy arena
  /*main_arena_manager.arena->next = malloc(sizeof(ho_arena_descriptor));
  if (main_arena_manager.arena->next == null)
    return -1;
  main_arena_manager.num_arenas = 2;
  main_arena_manager.arena->next->next = null;
  main_arena_manager.arena->next->initial_address = alloc(ARENA_SIZE, &(main_arena_manager.arena->next->id));

  for (aux=0; aux<BLOCKS_PER_ARENA/8; aux++)
    main_arena_manager.arena->next->block_status_bitmap[aux] = 0;*/

  ho_block* block = append_block(&main_text.block_container->blocks[0], 0);
  add_text_to_block(block, "This is a Test!");
  block = append_block(&main_text.block_container->blocks[1], 1);
  add_text_to_block(block, "Hoshoyo newbie.");
  block = append_block(&main_text.block_container->blocks[2], 2);
  add_text_to_block(block, "MDI cleaned RioT!");
  block = append_block(&main_text.block_container->blocks[2], 2);
  add_text_to_block(block, "ShowTime > FalleN");
  block = append_block(&main_text.block_container->blocks[2], 2);
  add_text_to_block(block, "I like cats.");
  block = append_block(&main_text.block_container->blocks[2], 2);
  add_text_to_block(block, "I like dogs.");
  block = append_block(&main_text.block_container->blocks[0], 0);
  add_text_to_block(block, "My computer is hot.");
  block = append_block(&main_text.block_container->blocks[6], 6);
  add_text_to_block(block, "I want a magazine.");
  block = append_block(&main_text.block_container->blocks[7], 7);
  add_text_to_block(block, "You dont know how to play.");
  block = append_block(&main_text.block_container->blocks[7], 7);
  add_text_to_block(block, "C is better than C++");
  block = append_block(&main_text.block_container->blocks[0], 0);
  add_text_to_block(block, "PHP is the best language in the world!");
  block = append_block(&main_text.block_container->blocks[7], 7);
  add_text_to_block(block, "eval() is powerful.");
  block = append_block(&main_text.block_container->blocks[7], 7);
  add_text_to_block(block, "HTML is not a programming language");
  block = append_block(&main_text.block_container->blocks[1], 1);
  add_text_to_block(block, "One fire bolt is enough to kill Hoshoyo. Proved by IhaHigorII.");
  block = append_block(&main_text.block_container->next->blocks[1], 7+2);
  add_text_to_block(block, "MDI cleaned Supremacia again.");
  print_text(main_text);

  print_arena_descriptor(*main_arena_manager.arena);

  // end of tests

  return 0;
}

ho_block* put_new_block_and_move_others_to_right(ho_block new_block, u32 array_position, ho_block_container* block_container, u32 absolute_position)
{
  u32 i;
  u32 current_array_position = array_position;
  ho_block_container* current_container = block_container;
  ho_block last_block = new_block;
  ho_block* new_block_inside_array = null;

  for (i=absolute_position; i<main_text.num_blocks; i++)
  {
    if ((current_array_position + 1) >= BLOCKS_PER_CONTAINER)
    {
      current_array_position = 0;

      if (current_container->next == null)
      {
        current_container->next = malloc(sizeof(ho_block_container));
        current_container->next->next = null;
      }

      current_container = current_container->next;
    }
    else
    {
      current_array_position++;
    }

    ho_block aux_block = current_container->blocks[current_array_position];
    current_container->blocks[current_array_position] = last_block;
    last_block = aux_block;

    if (i == absolute_position)
      new_block_inside_array = &(current_container->blocks[current_array_position]);
  }

  main_text.num_blocks++;

  return new_block_inside_array;
}

ho_block* append_block(ho_block* existing_block, u32 existing_block_absolute_position)
{
  u32 existing_block_array_position, new_block_array_position, new_number_of_blocks, new_block_absolute_position;
  ho_block_container* existing_block_container;
  ho_block_container* new_block_container;

  // test if existing block received as parameter matches its absolute position (also received as parameter)
  existing_block_container = main_text.block_container;
  for (existing_block_array_position=existing_block_absolute_position; existing_block_array_position>=BLOCKS_PER_CONTAINER; existing_block_array_position-=BLOCKS_PER_CONTAINER)
    existing_block_container = existing_block_container->next;

  ho_block* found_block = &existing_block_container->blocks[existing_block_array_position];

  if (found_block != existing_block)
  {
    printf("APPEND_BLOCK FUNCTION ERROR: BLOCKS DONT MATCH!");
    return null;
  }

  // fill new block
  ho_block new_block;
  new_block.total_size = BLOCK_SIZE;
  new_block.occupied = 0;
  new_block.empty = BLOCK_SIZE;
  new_block.data = request_new_block_data();

  // put new block in its position and move others blocks to right
  ho_block* new_block_inside_array = put_new_block_and_move_others_to_right(new_block, existing_block_array_position, existing_block_container, existing_block_absolute_position);

  // split logic
  // TO DO

  return new_block_inside_array;
}

void* fill_arena_bitmap_and_return_address(ho_arena_descriptor* arena_descriptor)
{
  u8 mask = 0x01, bitmap_byte = 0xFF;
  u32 i, position = 0, bitmap_array_position;

  for (i=0; i<BLOCKS_PER_ARENA/8; i++)
  {
    bitmap_byte = arena_descriptor->block_status_bitmap[i];
    if (bitmap_byte != 0xFF)
    {
      bitmap_array_position = i;
      break;
    }

    position += 8;
  }

  if (bitmap_byte == 0xFF)
    return null;

  for (i=0; i<8; i++)
  {
    if (!(bitmap_byte & mask))
    {
      arena_descriptor->block_status_bitmap[bitmap_array_position] |= mask;
      break;
    }
    mask = mask << 1;
  }

  position += i;

  return (void*)((char*)arena_descriptor->initial_address + (BLOCK_SIZE * position));

  //return main_arena_manager.arena->initial_address;
}

u8* request_new_block_data()
{
  u32 i;
  ho_arena_descriptor* current_arena;

  current_arena = main_arena_manager.arena;

  for (i=0; i<(main_arena_manager.num_arenas - 1); i++)
  {
    // fill exclusions logic
    // to do

    current_arena = current_arena->next;
  }

  return fill_arena_bitmap_and_return_address(current_arena);
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

  printf("\n");
}

void print_arena_descriptor(ho_arena_descriptor arena_descriptor)
{
  u32 i, j, k, array_position;
  const u32 blocks_per_line = 32; // must be a multiply of 8

  printf("ID: %u\n", arena_descriptor.id);
  printf("Initial Address: %p\n", arena_descriptor.initial_address);

  for (i=0, array_position = 0; i<BLOCKS_PER_ARENA; i += blocks_per_line)
  {
    for (j=0; j<blocks_per_line; j+=8, array_position++)
    {
      u8 blocks_bitmap = arena_descriptor.block_status_bitmap[array_position];

      for (k=0; k<8; k++)
      {
        if ((blocks_bitmap & (1 << k)))
          printf("O");  // occupied
        else
          printf("F");  // free
      }

      if ((i + j + 8) >= BLOCKS_PER_ARENA)
        break;
    }
    printf("\n");
  }
}

void print_text(ho_text text)
{
  u32 i, j;
  ho_block_container* block_container;
  printf("- TEXT -\n");
  printf("Num Blocks: %u\n", text.num_blocks);
  block_container = text.block_container;

  for (i=0; i<text.num_blocks; i+=BLOCKS_PER_CONTAINER)
  {
    for (j=0; j<BLOCKS_PER_CONTAINER; j++)
    {
      printf("\nBLOCK %u\n", i+j);
      print_block(block_container->blocks[j]);

      if ((i + j + 1) >= text.num_blocks)
        break;
    }

    block_container = block_container->next;
  }

  printf("\n");
}

void print_arena_manager(ho_arena_manager arena_manager)
{
  u32 aux;
  ho_arena_descriptor* desc_aux;
  printf("- ARENA MANAGER -\n");
  printf("Num Arenas: %u\n", arena_manager.num_arenas);
  desc_aux = arena_manager.arena;
  for (aux=0; aux<arena_manager.num_arenas; aux++)
  {
    printf("\nARENA %u:\n", aux+1);
    print_arena_descriptor(*desc_aux);
    desc_aux = desc_aux->next;
  }
}
