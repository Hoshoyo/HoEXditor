#include "text.h"
#include "util.h"
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

  for (aux=0; aux<size; ++aux)
    dest[aux] = src[aux];
}

u32 init_text()
{
  u32 aux;

  // First arena creation.
  main_arena_manager.arena = malloc(sizeof(ho_arena_descriptor));
  if (main_arena_manager.arena == null)
  {
    error_fatal("Error creating first arena. Out of space.", 0);
    return -1;
  }
  main_arena_manager.num_arenas = 1;
  main_arena_manager.arena->next = null;
  main_arena_manager.arena->initial_address = alloc(ARENA_SIZE, &(main_arena_manager.arena->id));
  main_arena_manager.arena->first_deleted_block = null;
  main_arena_manager.arena->last_deleted_block = null;

  for (aux=0; aux<BLOCKS_PER_ARENA/8; ++aux)
    main_arena_manager.arena->block_status_bitmap[aux] = 0;

  // First block creation.
  main_text.block_container = malloc(sizeof(ho_block_container));
  if (main_text.block_container == null)
  {
    error_fatal("Error creating first block. Out of space.", 0);
    return -1;
  }
  main_text.block_container->next = null;
  main_text.block_container->num_blocks_in_container = 1;
  main_text.block_container->blocks[0].total_size = BLOCK_SIZE;
  main_text.block_container->blocks[0].occupied = 0;
  main_text.block_container->blocks[0].empty = BLOCK_SIZE;
  main_text.block_container->blocks[0].container = main_text.block_container;
  main_text.block_container->blocks[0].position_in_container = 0;
  main_text.block_container->blocks[0].block_data.data = main_arena_manager.arena->initial_address;
  main_text.block_container->blocks[0].block_data.arena = main_arena_manager.arena;
  main_arena_manager.arena->block_status_bitmap[0] |= (1 << 0); // fill first arena bitmap
  main_text.num_blocks = 1;

  insert_text_in_block(&main_text.block_container->blocks[0], "Wow! This Is My First Block! How Nice!", 0,
    strlen("Wow! This Is My First Block! How Nice!"), false);

  // Code below: tests.
  /*ho_block* block = append_block(&main_text.block_container->blocks[0], 0);
  add_text_to_block(block, "This is a Test!");
  ho_block* new_block = append_block(&main_text.block_container->blocks[1], 1);
  split_block(block, new_block);*/

  ho_block* block = append_block(main_text.block_container->blocks[0]);
  insert_text_in_block(block, "This is a Test!", 0, strlen("This is a Test!"), false);
  block = append_block(main_text.block_container->blocks[1]);
  insert_text_in_block(block, "lolz", 0, strlen("lolz"), false);
  block = append_block(main_text.block_container->blocks[2]);
  insert_text_in_block(block, "I wonder how many bytes does this sentence have! I love you! :-)", 0,
  strlen("I wonder how many bytes does this sentence have! I love you! :-)"), false);
  insert_text_in_block(block, "Do you? ", strlen("I wonder how many bytes does this sentence have! I love you! "), strlen("Do you? "), true);
  insert_text_in_block(block, "It's amazing how society has evolved among the years. I would say that this is result of an organized protest against mediocrity.",
    0, strlen( "It's amazing how society has evolved along the years. I would say that this is result of an organized protest against mediocrity. "), true);
  block = append_block(main_text.block_container->blocks[2]);
  insert_text_in_block(block, "ShowTime > FalleN", 0, strlen("ShowTime > FalleN"), false);
  block = append_block(main_text.block_container->blocks[2]);
  insert_text_in_block(block, "I like cats.", 0, strlen("I like cats."), false);
  block = append_block(main_text.block_container->blocks[2]);
  insert_text_in_block(block, "I like dogs.", 0, strlen("I like dogs."), false);
  block = append_block(main_text.block_container->blocks[0]);
  ho_block* stored_block = block;
  insert_text_in_block(block, "My computer is hot.", 0, strlen("My computer is hot."), false);
  /*block = append_block(main_text.block_container->blocks[6]);
  insert_text_in_block(block, "I want a magazine.", 0, strlen("I want a magazine."), false);
  block = append_block(main_text.block_container->blocks[7]);
  insert_text_in_block(block, "You dont know how to play.", 0, strlen("You dont know how to play."), false);
  block = append_block(main_text.block_container->blocks[7]);
  insert_text_in_block(block, "C is better than C++", 0, strlen("C is better than C++"), false);
  block = append_block(main_text.block_container->blocks[0]);
  insert_text_in_block(block, "PHP is the best language in the world!", 0, strlen("PHP is the best language in the world!"), false);
  block = append_block(main_text.block_container->blocks[7]);
  insert_text_in_block(block, "eval() is powerful.", 0, strlen("eval() is powerful."), false);
  block = append_block(main_text.block_container->blocks[7]);
  insert_text_in_block(block, "HTML is not a programming language", 0, strlen("HTML is not a programming language"), false);
  block = append_block(main_text.block_container->blocks[1]);
  insert_text_in_block(block, "One fire bolt is enough to kill Hoshoyo. Proved by IhaHigorII. Fudeu ne.", 0,
   strlen("One fire bolt is enough to kill Hoshoyo. Proved by IhaHigorII. Fudeu ne."), true);
  block = append_block(main_text.block_container->next->blocks[1]);
  insert_text_in_block(block, "MDI cleaned Supremacia again.", 0, strlen("MDI cleaned Supremacia again."), false);*/

  print_text(main_text);
  delete_block_and_move_others_to_left(*stored_block);
  print_text(main_text);

  // simulate exclusion:
  /*const u32 block_number = 3;
  main_arena_manager.arena->first_deleted_block = malloc(sizeof(ho_deleted_block));
  main_arena_manager.arena->last_deleted_block = main_arena_manager.arena->first_deleted_block;
  main_arena_manager.arena->first_deleted_block->next = null;
  main_arena_manager.arena->first_deleted_block->block_number = block_number;
  main_arena_manager.arena->block_status_bitmap[0] &= ~(1 << block_number);
print_arena_manager(main_arena_manager);
  block = append_block(&main_text.block_container->next->blocks[1], 7+2);
  add_text_to_block(block, "Java is not verbose.");
  //print_text(main_text);
*/
  //print_text(main_text);
  //print_arena_manager(main_arena_manager);

  return 0;
}

ho_block* put_new_block_and_move_others_to_right(ho_block new_block, ho_block existing_block)
{
  u32 current_array_position = existing_block.position_in_container;
  ho_block_container* last_container;
  ho_block_container* current_container = existing_block.container;
  ho_block aux_block, last_block = new_block;
  u32 current_container_num_blocks = current_container->num_blocks_in_container - current_array_position - 1;

  // start loop to move blocks
  do
  {
    // move blocks until end of container
    for (; current_container_num_blocks > 0; --current_container_num_blocks)
    {
      ++current_array_position;
      aux_block = current_container->blocks[current_array_position];
      current_container->blocks[current_array_position] = last_block;
      last_block = aux_block;

      current_container->blocks[current_array_position].position_in_container = current_array_position;
      current_container->blocks[current_array_position].container = current_container;
    }

    // changes container or breaks the loop
    last_container = current_container;
    current_container = current_container->next;
    if (current_container != null)
    {
      current_array_position = -1;
      current_container_num_blocks = current_container->num_blocks_in_container;
    }
  } while (current_container != null);

  // move the last element, creating a new container if necessary
  if (last_container->num_blocks_in_container == BLOCKS_PER_CONTAINER)
  {
    ho_block_container* new_container = malloc(sizeof(ho_block_container));
    new_container->num_blocks_in_container = 0;
    new_container->next = null;
    last_container->next = new_container;
    current_array_position = 0;
    current_container = new_container;
  }
  else
  {
    ++current_array_position;
    current_container = last_container;
  }

  current_container->blocks[current_array_position] = last_block;
  current_container->blocks[current_array_position].position_in_container = current_array_position;
  current_container->blocks[current_array_position].container = current_container;

  ++current_container->num_blocks_in_container;
  ++main_text.num_blocks;

  // returns the new_block inside the array.
  if (existing_block.position_in_container == (BLOCKS_PER_CONTAINER - 1))
    return &existing_block.container->next->blocks[0];
  else
    return &existing_block.container->blocks[existing_block.position_in_container + 1];
}

void delete_block_and_move_others_to_left(ho_block block_to_be_deleted)
{
  ho_block_container* current_container = block_to_be_deleted.container;
  ho_block_container* last_container = current_container;
  u32 current_array_position = block_to_be_deleted.position_in_container;
  u32 current_container_num_blocks = current_container->num_blocks_in_container - current_array_position - 1;

  // start loop to move blocks
  do
  {
    // move blocks until end of container
    for (; current_container_num_blocks > 0; --current_container_num_blocks)
    {
      current_container->blocks[current_array_position] = current_container->blocks[current_array_position + 1];

      current_container->blocks[current_array_position].position_in_container = current_array_position;
      current_container->blocks[current_array_position].container = current_container;

      ++current_array_position;
    }

    // changes container or breaks the loop
    last_container = current_container;
    current_container = current_container->next;
    if (current_container != null)
    {
      last_container->blocks[last_container->num_blocks_in_container - 1] = current_container->blocks[0];
      last_container->blocks[last_container->num_blocks_in_container - 1].position_in_container = last_container->num_blocks_in_container - 1;
      last_container->blocks[last_container->num_blocks_in_container - 1].container = last_container;

      current_array_position = 0;
      current_container_num_blocks = current_container->num_blocks_in_container - 1;
    }
  } while (current_container != null);

  current_container = last_container;

  --current_container->num_blocks_in_container;
  --main_text.num_blocks;

  free_block_data(block_to_be_deleted.block_data);
}

ho_block* append_block(ho_block existing_block)
{
  // fill new block
  ho_block new_block;
  new_block.total_size = BLOCK_SIZE;
  new_block.occupied = 0;
  new_block.empty = BLOCK_SIZE;
  new_block.block_data = request_new_block_data();

  // put new block in its position and move others blocks to right
  ho_block* new_block_inside_array = put_new_block_and_move_others_to_right(new_block, existing_block);

  return new_block_inside_array;
}

void split_block(ho_block* block_to_be_split, ho_block* new_block)
{
  u32 remaining_text_size = block_to_be_split->occupied/2;
  u32 new_text_size = block_to_be_split->occupied - remaining_text_size;

  copy_string(new_block->block_data.data, block_to_be_split->block_data.data + remaining_text_size, new_text_size);

  block_to_be_split->occupied -= new_text_size;
  block_to_be_split->empty += new_text_size;
  new_block->occupied += new_text_size;
  new_block->empty -= new_text_size;
}

u32 insert_text_in_block(ho_block* block, u8* text, u32 data_position, u32 text_size, bool split_if_necessary)
{
  // tests if it must be split
  if (text_size > block->empty)
  {
    if (!split_if_necessary)
      return -1;

    u8* end_of_text_backup = null;
    u32 size_of_text_after_data_position = block->occupied - data_position;

    // if there's text after data_position, the text is copied, so it can be placed correctly later.
    if (size_of_text_after_data_position > 0)
    {
      end_of_text_backup = malloc(size_of_text_after_data_position * sizeof(u8));
      copy_string(end_of_text_backup, block->block_data.data + data_position, size_of_text_after_data_position);
      block->occupied -= size_of_text_after_data_position;
      block->empty += size_of_text_after_data_position;
    }

    u32 size_of_text_left = text_size;
    ho_block* current_block = block;

    // start copying the text to the block, splitting when necessary.
    while (size_of_text_left > 0)
    {
      // copy the text, making the block become full if not last loop
      u32 size_of_text_to_be_moved = (size_of_text_left > current_block->empty) ? current_block->empty : size_of_text_left;
      copy_string(current_block->block_data.data + current_block->occupied, text + text_size - size_of_text_left, size_of_text_to_be_moved);
      current_block->occupied += size_of_text_to_be_moved;
      current_block->empty -= size_of_text_to_be_moved;
      size_of_text_left -= size_of_text_to_be_moved;

      // split, if necessary - it will split every loop except the last one
      if (size_of_text_left > 0)
      {
        ho_block* new_block = append_block(*current_block);
        split_block(current_block, new_block);

        current_block = new_block;
      }
    }

    // test if the text that was after data_position fits current_block. If not, split it.
    if (size_of_text_after_data_position > current_block->empty)
    {
      ho_block* new_block = append_block(*current_block);
      split_block(current_block, new_block);
      current_block = new_block;
    }

    // restore the piece of text that was after data_position
    copy_string(current_block->block_data.data + current_block->occupied, end_of_text_backup, size_of_text_after_data_position);
    current_block->occupied += size_of_text_after_data_position;
    current_block->empty -= size_of_text_after_data_position;

    if (end_of_text_backup != null)
      free(end_of_text_backup);
  }
  else
  {
    u32 size_of_text_after_data_position = block->occupied - data_position;

    // if there's text after data_position, the text is moved to the end of the block, giving space to the new text.
    if (size_of_text_after_data_position > 0)
      copy_string(block->block_data.data + data_position + text_size, block->block_data.data + data_position, size_of_text_after_data_position);

    // copy the new text to its location.
    copy_string(block->block_data.data + data_position, text, text_size);

    // refresh block's attributes
    block->occupied += text_size;
    block->empty -= text_size;
  }

  return 0;
}

void* fill_arena_bitmap_and_return_address(ho_arena_descriptor* arena_descriptor)
{
  u8 mask = 0x01, bitmap_byte = 0xFF;
  u32 i, position = 0, bitmap_array_position;

  for (i=0; i<BLOCKS_PER_ARENA/8; ++i)
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

  for (i=0; i<8; ++i)
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
}

ho_block_data request_new_block_data()
{
  u32 i;
  ho_arena_descriptor* current_arena;
  ho_block_data block_data;

  current_arena = main_arena_manager.arena;

  for (i=0; i<(main_arena_manager.num_arenas - 1); ++i)
  {
    // fill exclusions logic
    if (current_arena->first_deleted_block != null)
    {
      u32 block_number = current_arena->first_deleted_block->block_number;
      u8 bitmap_byte = current_arena->block_status_bitmap[(int)(block_number / 8)];
      u8 mask = 1 << (block_number % 8);
      if (bitmap_byte & mask)
      {
        error_fatal("Error requesting new block data: Deleted block is being used in corresponding bitmap.", 0);
        block_data.data = null;
        block_data.arena = null;
        return block_data;
      }
      else
      {
        current_arena->block_status_bitmap[(int)(block_number / 8)] |= mask;
        ho_deleted_block* deleted_block = current_arena->first_deleted_block;
        current_arena->first_deleted_block = current_arena->first_deleted_block->next;
        if (current_arena->last_deleted_block == current_arena->first_deleted_block)
          current_arena->last_deleted_block = null;
        free(deleted_block);

        block_data.data = (void*)((char*)current_arena->initial_address + (BLOCK_SIZE * block_number));
        block_data.arena = current_arena;

        return block_data;
      }
    }

    current_arena = current_arena->next;
  }

  void* address = fill_arena_bitmap_and_return_address(current_arena);

  if (address != null)
  {
    block_data.data = address;
    block_data.arena = current_arena;
  }
  else
  {
    // if address is null, arena is full. A new arena must be created.
    ho_arena_descriptor* new_arena = create_new_arena(current_arena);

    // new arena's first block is taken and returned.
    new_arena->block_status_bitmap[0] = (1 << 0);

    block_data.data = new_arena->initial_address;
    block_data.arena = new_arena;
  }

  return block_data;
}

void free_block_data(ho_block_data block_data)
{
  // to do
}

ho_arena_descriptor* create_new_arena(ho_arena_descriptor* last_arena)
{
  u32 aux;

  if (last_arena->next != null)
  {
    error_fatal("Error creating new Arena. Inconsistency detected: 'Next' pointer is not null.", 0);
    return null;
  }

  last_arena->next = malloc(sizeof(ho_arena_descriptor));
  if (last_arena->next == null)
  {
    error_fatal("Error creating new Arena. Out of space.", 0);
    return null;
  }
  ++main_arena_manager.num_arenas;
  last_arena->next->next = null;
  last_arena->next->initial_address = alloc(ARENA_SIZE, &(last_arena->next->id));
  last_arena->next->first_deleted_block = null;
  last_arena->next->last_deleted_block = null;

  if (last_arena->next->initial_address == null)
  {
    error_fatal("Error creating new Arena. Arena could not be allocated.", 0);
    return null;
  }

  for (aux=0; aux<BLOCKS_PER_ARENA/8; ++aux)
    last_arena->next->block_status_bitmap[aux] = 0;

  return last_arena->next;
}

void print_block(ho_block block)
{
  u32 aux;

  printf("Total Size: %u\n", block.total_size);
  printf("Occupied: %u\n", block.occupied);
  printf("Empty: %u\n", block.empty);
  printf("Container Address: %p\n", block.container);
  printf("Position in Container: %u\n", block.position_in_container);
  printf("Data: ");

  for (aux=0; aux<block.occupied; ++aux)
    printf("%c", *(block.block_data.data + aux));

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
    for (j=0; j<blocks_per_line; j+=8, ++array_position)
    {
      u8 blocks_bitmap = arena_descriptor.block_status_bitmap[array_position];

      for (k=0; k<8; ++k)
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
    printf("\nBLOCK CONTAINER %u", i/BLOCKS_PER_CONTAINER);
    printf("\nNumber of Blocks in Container: %u\n", block_container->num_blocks_in_container);
    for (j=0; j<BLOCKS_PER_CONTAINER; ++j)
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
  for (aux=0; aux<arena_manager.num_arenas; ++aux)
  {
    printf("\nARENA %u:\n", aux+1);
    print_arena_descriptor(*desc_aux);
    desc_aux = desc_aux->next;
  }
}
