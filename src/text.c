#include "text.h"
#include "util.h"
#include "memory.h"

/* BLOCK TEXT GLOBAL INFO */
bool _t_texts_valid[MAX_FILES_OPEN] = {false};
ho_text* _t_texts[MAX_FILES_OPEN] = {0};
internal ho_arena_manager* _t_arenas[MAX_FILES_OPEN] = {0};
/* ********************** */

/* CONTIGUOUS TEXT GLOBAL INFO */
bool _t_contiguous_text_valid[MAX_CONTIGUOUS_TEXT_OPEN] = {false};
/* *************************** */

s32 get_empty_id(bool is_block_text)
{
  u32 aux;

  if (is_block_text)
  {
    for (aux=0; aux<MAX_FILES_OPEN; ++aux)
      if (!_t_texts_valid[aux])
        return aux;
  }
  else
  {
    for (aux=0; aux<MAX_CONTIGUOUS_TEXT_OPEN; ++aux)
      if (!_t_contiguous_text_valid[aux])
        return aux;
  }

  return -1;
}

s32 init_text_backbone(text_id* tid, bool is_block_text)
{
  u32 aux;

  if (is_block_text)
  {
    tid->id = get_empty_id(true);
    tid->is_block_text = true;
    if (tid->id < 0)  return -1;

    _t_texts_valid[tid->id] = true;

    // First arena creation.
    _t_arenas[tid->id] = halloc(sizeof(ho_arena_manager));
    _t_arenas[tid->id]->arena = halloc(sizeof(ho_arena_descriptor));
    if (_t_arenas[tid->id]->arena == null)
    {
      error_fatal("Error creating first arena. Out of space.", 0);
      return -1;
    }
    _t_arenas[tid->id]->num_arenas = 1;
    _t_arenas[tid->id]->arena->next = null;
    _t_arenas[tid->id]->arena->previous = null;
    _t_arenas[tid->id]->arena->initial_address = alloc(ARENA_SIZE, &(_t_arenas[tid->id]->arena->id));

    for (aux=0; aux<BLOCKS_PER_ARENA/8; ++aux)
      _t_arenas[tid->id]->arena->block_status_bitmap[aux] = 0;

    // First block creation.
    _t_texts[tid->id] = halloc(sizeof(ho_text));
    _t_texts[tid->id]->block_container = halloc(sizeof(ho_block_container));
    if (_t_texts[tid->id]->block_container == null)
    {
      error_fatal("Error creating first block. Out of space.", 0);
      return -1;
    }
    _t_texts[tid->id]->block_container->next = null;
    _t_texts[tid->id]->block_container->previous = null;
    _t_texts[tid->id]->block_container->num_blocks_in_container = 1;
    _t_texts[tid->id]->block_container->total_occupied = 0;
    _t_texts[tid->id]->block_container->blocks[0].total_size = BLOCK_SIZE;
    _t_texts[tid->id]->block_container->blocks[0].occupied = 0;
    _t_texts[tid->id]->block_container->blocks[0].empty = BLOCK_SIZE;
    _t_texts[tid->id]->block_container->blocks[0].container = _t_texts[tid->id]->block_container;
    _t_texts[tid->id]->block_container->blocks[0].position_in_container = 0;
    _t_texts[tid->id]->block_container->blocks[0].block_data.data = _t_arenas[tid->id]->arena->initial_address;
    _t_texts[tid->id]->block_container->blocks[0].block_data.arena = _t_arenas[tid->id]->arena;
    _t_arenas[tid->id]->arena->block_status_bitmap[0] |= (1 << 0); // fill first arena bitmap
    _t_texts[tid->id]->num_blocks = 1;
  }
  else
  {
    tid->id = get_empty_id(false);
    tid->is_block_text = false;
    if (tid->id < 0)  return -1;

    _t_contiguous_text_valid[tid->id] = true;
  }

  log_success("\nText backbone successfully initializated.\n");
  return 0;
}

s32 destroy_text_backbone(text_id tid)
{
  if (tid.is_block_text)
  {
    u32 i;
    ho_arena_descriptor* arena_descriptor = _t_arenas[tid.id]->arena;
    ho_block_container* block_container = _t_texts[tid.id]->block_container;
    ho_block_container* aux2;

    u32 num_arenas = _t_arenas[tid.id]->num_arenas;

    // free arenas
    while (arena_descriptor != null)
    {
      free_arena(tid, arena_descriptor);
      arena_descriptor = _t_arenas[tid.id]->arena;
    }

    // free block_containers
    while (block_container != null)
    {
      aux2 = block_container->next;
      hfree(block_container);
      block_container = aux2;
    }

    hfree(_t_texts[tid.id]);
    hfree(_t_arenas[tid.id]);
    _t_texts[tid.id] = null;
    _t_arenas[tid.id] = null;

    _t_texts_valid[tid.id] = false;
  }
  else
  {
    _t_contiguous_text_valid[tid.id] = false;
  }

  log_success("\nText backbone successfully released.\n");

  return 0;
}

ho_block* put_new_block_and_move_others_to_right(text_id tid, ho_block new_block, ho_block existing_block)
{
  if (!tid.is_block_text)
    return null;

  u32 current_array_position = existing_block.position_in_container;
  ho_block_container* last_container;
  ho_block_container* current_container = existing_block.container;
  ho_block aux_block, last_block = new_block;
  u32 current_container_num_blocks = current_container->num_blocks_in_container - current_array_position - 1;
  bool first_block_in_container = true;

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

      if (first_block_in_container)
      {
        current_container->total_occupied += current_container->blocks[current_array_position].occupied;
        first_block_in_container = false;
      }
    }

    // changes container or breaks the loop
    last_container = current_container;
    current_container = current_container->next;
    last_container->total_occupied -= last_block.occupied;

    if (current_container != null)
    {
      current_array_position = -1;
      current_container_num_blocks = current_container->num_blocks_in_container;
      first_block_in_container = true;
    }
  } while (current_container != null);

  // move the last element, creating a new container if necessary
  if (last_container->num_blocks_in_container == BLOCKS_PER_CONTAINER)
  {
    ho_block_container* new_container = halloc(sizeof(ho_block_container));
    new_container->num_blocks_in_container = 0;
    new_container->total_occupied = 0;
    new_container->next = null;
    new_container->previous = last_container;
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
  current_container->total_occupied += last_block.occupied;
  ++_t_texts[tid.id]->num_blocks;

  // returns the new_block inside the array.
  if (existing_block.position_in_container == (BLOCKS_PER_CONTAINER - 1))
    return &existing_block.container->next->blocks[0];
  else
    return &existing_block.container->blocks[existing_block.position_in_container + 1];
}

ho_block* delete_block_and_move_others_to_left(text_id tid, ho_block block_to_be_deleted)
{
  if (!tid.is_block_text)
    return null;

  ho_block_container* current_container = block_to_be_deleted.container;
  ho_block_container* last_container = current_container;
  u32 current_array_position = block_to_be_deleted.position_in_container;
  u32 current_container_num_blocks = current_container->num_blocks_in_container - current_array_position - 1;

  ho_block* position = &current_container->blocks[current_array_position];

  current_container->total_occupied -= block_to_be_deleted.occupied;

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
      last_container->total_occupied += current_container->blocks[0].occupied;
      current_container->total_occupied -= current_container->blocks[0].occupied;

      current_array_position = 0;
      current_container_num_blocks = current_container->num_blocks_in_container - 1;
    }
  } while (current_container != null);

  --last_container->num_blocks_in_container;
  --_t_texts[tid.id]->num_blocks;

  // if last container have 0 blocks, it should be deleted.
  if (last_container->num_blocks_in_container == 0)
  {
    // if previous != null, it should be deleted. if previous == null, this is the first block container, so it probably should not be deleted
    if (last_container->previous != null)
    {
      last_container->previous->next = null;
      hfree(last_container);
    }
  }

  free_block_data(tid, block_to_be_deleted.block_data);

  return position;
}

ho_block* append_block(text_id tid, ho_block existing_block)
{
  if (!tid.is_block_text)
    return null;

  // fill new block
  ho_block new_block;
  new_block.total_size = BLOCK_SIZE;
  new_block.occupied = 0;
  new_block.empty = BLOCK_SIZE;
  new_block.block_data = request_new_block_data(tid);

  // put new block in its position and move others blocks to right
  ho_block* new_block_inside_array = put_new_block_and_move_others_to_right(tid, new_block, existing_block);

  return new_block_inside_array;
}

ho_block* delete_block(text_id tid, ho_block block_to_be_deleted)
{
  if (!tid.is_block_text)
    return null;

  if (_t_texts[tid.id]->num_blocks > 1)
    return delete_block_and_move_others_to_left(tid, block_to_be_deleted);

  return null;
}

void split_block(ho_block* block_to_be_split, ho_block* new_block)
{
  u32 remaining_text_size = block_to_be_split->occupied/2;
  u32 new_text_size = block_to_be_split->occupied - remaining_text_size;

  copy_string(new_block->block_data.data, block_to_be_split->block_data.data + remaining_text_size, new_text_size);

  block_to_be_split->occupied -= new_text_size;
  block_to_be_split->empty += new_text_size;
  block_to_be_split->container->total_occupied -= new_text_size;
  new_block->occupied += new_text_size;
  new_block->empty -= new_text_size;
  new_block->container->total_occupied += new_text_size;
}

u32 insert_text_in_block(text_id tid, ho_block* block, u8* text, u32 data_position, u32 text_size, bool split_if_necessary)
{
  if (!tid.is_block_text)
    return -1;

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
      end_of_text_backup = halloc(size_of_text_after_data_position * sizeof(u8));
      copy_string(end_of_text_backup, block->block_data.data + data_position, size_of_text_after_data_position);
      block->occupied -= size_of_text_after_data_position;
      block->empty += size_of_text_after_data_position;
      block->container->total_occupied -= size_of_text_after_data_position;
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
      current_block->container->total_occupied += size_of_text_to_be_moved;
      size_of_text_left -= size_of_text_to_be_moved;

      // split, if necessary - it will split every loop except the last one
      if (size_of_text_left > 0)
      {
        ho_block* new_block = append_block(tid, *current_block);
        split_block(current_block, new_block);

        current_block = new_block;
      }
    }

    // test if the text that was after data_position fits current_block. If not, split it.
    if (size_of_text_after_data_position > current_block->empty)
    {
      ho_block* new_block = append_block(tid, *current_block);
      split_block(current_block, new_block);
      current_block = new_block;
    }

    // restore the piece of text that was after data_position
    copy_string(current_block->block_data.data + current_block->occupied, end_of_text_backup, size_of_text_after_data_position);
    current_block->occupied += size_of_text_after_data_position;
    current_block->empty -= size_of_text_after_data_position;
    current_block->container->total_occupied += size_of_text_after_data_position;

    if (end_of_text_backup != null)
      hfree(end_of_text_backup);
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
    block->container->total_occupied += text_size;
  }

  return 0;
}

u32 delete_text_in_block(text_id tid, ho_block* block, u32 data_position, u64 text_size, bool recursive_if_necessary)
{
  if (!tid.is_block_text)
    return -1;

  u32 current_data_position = data_position;
  u64 text_size_left = text_size;
  ho_block* last_block = block;

  while (last_block->occupied <= (current_data_position + text_size_left))
  {
    u32 size_to_delete = last_block->occupied - current_data_position;
    last_block->occupied -= size_to_delete;
    last_block->empty += size_to_delete;
    last_block->container->total_occupied -= size_to_delete;

    current_data_position = 0;
    text_size_left -= size_to_delete;

    if (last_block->occupied == 0)
      last_block = delete_block(tid, *last_block);
    else
    {
      if (text_size_left > 0)
      {
        if (last_block->position_in_container == BLOCKS_PER_CONTAINER - 1)
          last_block = &last_block->container->next->blocks[0];
        else
          last_block = &last_block->container->blocks[last_block->position_in_container + 1];
      }
    }

    if (text_size_left == 0)
      return 0;
  }

  copy_string(last_block->block_data.data + current_data_position, last_block->block_data.data + current_data_position + text_size_left, last_block->occupied - (current_data_position + text_size_left));
  last_block->occupied -= text_size_left;
  last_block->empty += text_size_left;
  last_block->container->total_occupied -= text_size_left;

  if (last_block->occupied == 0)
    delete_block(tid, *last_block);

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

ho_block_data request_new_block_data(text_id tid)
{
  if (!tid.is_block_text)
  {
    ho_block_data block_data = {.data = null, .arena = null};
    return block_data;
  }

  u32 i;
  ho_arena_descriptor* current_arena;
  ho_block_data block_data;
  void* address;

  current_arena = _t_arenas[tid.id]->arena;

  do
  {
    address = fill_arena_bitmap_and_return_address(current_arena);

    if (address == null)
    {
      if (current_arena->next != null)
        current_arena = current_arena->next;
      else
        break;
    }
  } while (address == null);


  if (address != null)
  {
    block_data.data = address;
    block_data.arena = current_arena;
  }
  else
  {
    // if address is null, arena is full. A new arena must be created.
    ho_arena_descriptor* new_arena = create_new_arena(tid, current_arena);

    // new arena's first block is taken and returned.
    new_arena->block_status_bitmap[0] = (1 << 0);

    block_data.data = new_arena->initial_address;
    block_data.arena = new_arena;
  }

  return block_data;
}

bool is_arena_empty(ho_arena_descriptor* arena)
{
  u32 i;

  for(i=0; i<BLOCKS_PER_ARENA/8; i++)
    if (arena->block_status_bitmap[i] != 0x00)
      return false;

  return true;
}

void free_arena(text_id tid, ho_arena_descriptor* arena)
{
  if (!tid.is_block_text)
    return;

  // adjust previous and next to point to each other
  if (arena->previous != null)
  {
    if (arena->next != null)
      arena->next->previous = arena->previous;

    arena->previous->next = arena->next;
  }
  else
  {
    // if arena->previous is null, it means that the arena that is being deleted is the first arena.
    if (arena->next != null)
      arena->next->previous = null;

    _t_arenas[tid.id]->arena = arena->next;
  }

  --_t_arenas[tid.id]->num_arenas;

  // free arena
  release(arena->id);

  // free arena_descriptor struct
  hfree(arena);
}

void free_block_data(text_id tid, ho_block_data block_data)
{
  if (!tid.is_block_text)
    return;

  ho_arena_descriptor* arena = block_data.arena;
  u32 difference = (u32)(block_data.data - (u8*)arena->initial_address);
  u32 arena_position = difference / BLOCK_SIZE;

  u8 bitmap_byte = arena->block_status_bitmap[arena_position / 8];
  u8 byte_position = arena_position % 8;

  if (bitmap_byte & (1 << byte_position))
  {
    arena->block_status_bitmap[arena_position / 8] &= ~(1 << byte_position);  // clear bit

    // if arena is empty, the whole arena is deleted.
    if (is_arena_empty(arena))
      free_arena(tid, arena);
  }
  else
  {
    error_fatal("free_block_data error: block was free in bitmap.", 0);
    return;
  }
}

ho_arena_descriptor* create_new_arena(text_id tid, ho_arena_descriptor* last_arena)
{
  if (!tid.is_block_text)
    return null;

  u32 aux;

  if (last_arena->next != null)
  {
    error_fatal("Error creating new Arena. Inconsistency detected: 'Next' pointer is not null.", 0);
    return null;
  }

  last_arena->next = halloc(sizeof(ho_arena_descriptor));
  if (last_arena->next == null)
  {
    error_fatal("Error creating new Arena. Out of space.", 0);
    return null;
  }
  ++_t_arenas[tid.id]->num_arenas;
  last_arena->next->next = null;
  last_arena->next->previous = last_arena;
  last_arena->next->initial_address = alloc(ARENA_SIZE, &(last_arena->next->id));

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

  print("Total Size: %u\n", block.total_size);
  print("Occupied: %u\n", block.occupied);
  print("Empty: %u\n", block.empty);
  print("Container Address: %p\n", block.container);
  print("Position in Container: %u\n", block.position_in_container);
  print("Arena First Address: %p\n", block.block_data.arena->initial_address);
  print("Data: ");

  for (aux=0; aux<block.occupied; ++aux)
    print("%c", *(block.block_data.data + aux));

  print("\n");
}

void print_arena_descriptor(ho_arena_descriptor arena_descriptor)
{
  u32 i, j, k, array_position;
  const u32 blocks_per_line = 32; // must be a multiply of 8

  print("ID: %u\n", arena_descriptor.id);
  print("Initial Address: %p\n", arena_descriptor.initial_address);

  for (i=0, array_position = 0; i<BLOCKS_PER_ARENA; i += blocks_per_line)
  {
    for (j=0; j<blocks_per_line; j+=8, ++array_position)
    {
      u8 blocks_bitmap = arena_descriptor.block_status_bitmap[array_position];

      for (k=0; k<8; ++k)
      {
        if ((blocks_bitmap & (1 << k)))
          print("O");  // occupied
        else
          print("F");  // free
      }

      if ((i + j + 8) >= BLOCKS_PER_ARENA)
        break;
    }
    print("\n");
  }
}

void print_text(ho_text text)
{
  u32 i, j;
  ho_block_container* block_container;
  print("- TEXT -\n");
  print("Num Blocks: %u\n", text.num_blocks);
  block_container = text.block_container;

  for (i=0; i<text.num_blocks; i+=BLOCKS_PER_CONTAINER)
  {
    print("\nBLOCK CONTAINER %u", i/BLOCKS_PER_CONTAINER);
    print("\nNumber of Blocks in Container: %u\n", block_container->num_blocks_in_container);
    print("\nTotal Container Ocuppied: %u\n", block_container->total_occupied);
    for (j=0; j<BLOCKS_PER_CONTAINER; ++j)
    {
      print("\nBLOCK %u\n", i+j);
      print_block(block_container->blocks[j]);

      if ((i + j + 1) >= text.num_blocks)
        break;
    }

    block_container = block_container->next;
  }

  print("\n");
}

void print_arena_manager(ho_arena_manager arena_manager)
{
  u32 aux;
  ho_arena_descriptor* desc_aux;
  print("- ARENA MANAGER -\n");
  print("Num Arenas: %u\n", arena_manager.num_arenas);
  desc_aux = arena_manager.arena;
  for (aux=0; aux<arena_manager.num_arenas; ++aux)
  {
    print("\nARENA %u:\n", aux+1);
    print_arena_descriptor(*desc_aux);
    desc_aux = desc_aux->next;
  }
}

bool check_text_backbone(text_id tid, u64 text_size)
{
  if (!tid.is_block_text)
    return false;

  u32 i, container_number = 0;
  u32 total_num_blocks = 0;
  u32 total_container_occupied = 0;
  u32 total_occupied = 0;
  ho_block_container* current_container = _t_texts[tid.id]->block_container;
  ho_block_container* previous_container = null;
  bool error_detected = false;

  while (current_container != null)
  {
    if (current_container->previous != previous_container)
    {
        print("Error in container %u: pointer to previous container is wrong\n", container_number);
        error_detected = true;
    }

    for (i=0; i<current_container->num_blocks_in_container; ++i)
    {
      if (current_container->blocks[i].occupied == 0)
      {
        print("Error in container %u, block %u: block is empty\n", container_number, i);
        error_detected = true;
      }

      if (current_container->blocks[i].block_data.arena == null)
      {
        print("Error in container %u, block %u: block's arena point to null.\n", container_number, i);
        error_detected = true;
      }

      if (current_container->blocks[i].block_data.data == null)
      {
        print("Error in container %u, block %u: block's arena point to null.\n", container_number, i);
        error_detected = true;
      }

      if (current_container->blocks[i].block_data.arena->initial_address > current_container->blocks[i].block_data.data)
      {
        print("Error in container %u, block %u: block's data pointer is below arena's first address in memory.\n", container_number, i);
        error_detected = true;
      }

      u64 difference = current_container->blocks[i].block_data.data - current_container->blocks[i].block_data.arena->initial_address;

      if (difference % BLOCK_SIZE != 0)
      {
        print("Error in container %u, block %u: block's data pointer is not aligned to arena's first address\n", container_number, i);
        error_detected = true;
      }

      if (difference > BLOCK_SIZE*BLOCKS_PER_ARENA)
      {
        print("Error in container %u, block %u: block's data pointer is higher than arena's max address\n", container_number, i);
        error_detected = true;
      }

      if (current_container->blocks[i].total_size != current_container->blocks[i].occupied + current_container->blocks[i].empty)
      {
        print("Error in container %u, block %u: total_size != occupied + empty\n", container_number, i);
        error_detected = true;
      }

      if (current_container->blocks[i].position_in_container != i)
      {
        print("Error in container %u, block %u: position_in_container != real position in container\n", container_number, i);
        error_detected = true;
      }

      if (current_container != current_container->blocks[i].container)
      {
        print("Error in container %u, block %u: container pointer != real container pointer\n", container_number, i);
        error_detected = true;
      }

      total_container_occupied += current_container->blocks[i].occupied;
    }

    if (total_container_occupied != current_container->total_occupied)
    {
      print("Error in container %u: container total_occupied != container blocks occupied ", container_number);
      print("(Container Occupied: %u, Blocks Occupied: %u)\n", current_container->total_occupied, total_container_occupied);
      error_detected = true;
    }

    total_occupied += total_container_occupied;
    total_container_occupied = 0;
    total_num_blocks += current_container->num_blocks_in_container;
    previous_container = current_container;
    current_container = current_container->next;
    ++container_number;
  }

  if (total_num_blocks != _t_texts[tid.id]->num_blocks)
  {
    print("Error in _t_texts[%d]: total number of blocks is wrong.\n", tid.id);
    error_detected = true;
  }

  if (total_occupied != text_size)
  {
    printf("Error: Sum of blocks 'occupied' differs from text size\n");
    error_detected = true;
  }

  return error_detected;
}

bool check_arena_backbone(text_id tid)
{
  if (!tid.is_block_text)
    return false;

  u32* ids = halloc(_t_arenas[tid.id]->num_arenas * sizeof(u32));
  ho_arena_descriptor* current_arena = _t_arenas[tid.id]->arena;
  ho_arena_descriptor* last_arena = null;
  u32 current_number_of_arenas = 0;
  bool error_detected = false;
  u32 number_of_blocks = 0;

  while (current_arena != null)
  {
    if (current_arena->previous != last_arena)
    {
      print("Error in arena %u: previous pointer is wrong\n", current_number_of_arenas);
      error_detected = true;
    }

    for (u32 i=0; i<BLOCKS_PER_ARENA/8; i++)
    {
      for (u32 j=0; j<8; j++)
      {
        if (current_arena->block_status_bitmap[i] & (1 << j))
        {
          ++number_of_blocks;
        }
      }
    }

    ids[current_number_of_arenas] = current_arena->id;
    ++current_number_of_arenas;
    last_arena = current_arena;
    current_arena = current_arena->next;
  }

  if (current_number_of_arenas != _t_arenas[tid.id]->num_arenas)
  {
    print("Error in _t_arenas[%d]: num_arenas != real number of arenas\n", tid.id);
    error_detected = true;
  }

  for (u32 i=0; i<_t_arenas[tid.id]->num_arenas; ++i)
  {
    u32 cid = ids[i];

    for (u32 j=i+1; j<_t_arenas[tid.id]->num_arenas; ++j)
    {
      if (cid == ids[j])
      {
        printf("Error in _t_arenas[%d]: arenas have the same id: %u\n", tid.id, cid);
        error_detected = true;
      }
    }
  }

  if (number_of_blocks != _t_texts[tid.id]->num_blocks)
  {
    printf("Error in _t_arenas[%d]: number of blocks in arenas are different from _t_texts[%d] number of blocks.\n", tid.id, tid.id);
    error_detected = true;
  }

  hfree(ids);

  return error_detected;
}
