#include "text_manager.h"
#include "memory.h"
#include "util.h"

internal u8* _tm_buffer;
internal u64 _tm_buffer_size;
internal u64 _tm_cursor_begin;
u64 _tm_text_size;
u64 _tm_valid_bytes;

s32 init_text_api(u8* filename)
{
  init_text();

  _tm_buffer = null;
  _tm_cursor_begin = 0;
  _tm_buffer_size = 0;
  _tm_text_size = 0;
  _tm_valid_bytes = 0;

  if (filename)
  {
    s64 size;
    u8* filedata = read_entire_file(filename, &size);

    u32 block_fill_value = (u32)(BLOCK_FILL_RATIO * BLOCK_SIZE);

    if (size > 0)
    {
      _tm_text_size = size;
      fill_blocks_with_text(filedata, size, block_fill_value);

      print_text(main_text);
    }
    else
    {
      _tm_text_size = 0;
      return -1;
    }
  }
  else
  {
    return -1;
  }

  return 0;
}

void fill_blocks_with_text(u8* data, s64 data_size, u32 block_fill_value)
{
  u64 data_remaining = data_size;
  u8* data_pointer = data;

  ho_block* last_block = &(main_text.block_container->blocks[0]);

  // first block must be populated individually.
  if (data_remaining > block_fill_value)
  {
    insert_text_in_block(last_block, data_pointer, 0, block_fill_value, false);
    data_pointer += block_fill_value;
    data_remaining -= block_fill_value;
  }
  else
  {
    insert_text_in_block(last_block, data_pointer, 0, data_remaining, false);
    data_pointer += data_remaining;
    data_remaining -= data_remaining;
  }

  // populates all other blocks, except the last one.
  while (data_remaining >= block_fill_value)
  {
    last_block = append_block(*last_block);
    insert_text_in_block(last_block, data_pointer, 0, block_fill_value, false);
    data_pointer += block_fill_value;
    data_remaining -= block_fill_value;
  }

  if (data_remaining > 0)
  {
    // populates last block
    last_block = append_block(*last_block);
    insert_text_in_block(last_block, data_pointer, 0, data_remaining, false);
    data_pointer += data_remaining;
    data_remaining -= data_remaining;
  }
}

s32 end_text_api()
{
  hfree(_tm_buffer);
  return destroy_text();
}

u8* get_text_buffer(u64 size, u64 cursor_begin)
{
  u32 ret;

  if (_tm_buffer == null)
  {
    _tm_buffer = halloc(sizeof(u8) * size);
  }
  else if (_tm_buffer_size != size)
  {
    hfree(_tm_buffer);
    _tm_buffer = halloc(sizeof(u8) * size);
  }

  _tm_buffer_size = size;
  _tm_cursor_begin = cursor_begin;

  ret = fill_buffer();

  if (!ret)
    return _tm_buffer;
  else
  {
    hfree(_tm_buffer);
    return null;
  }
}

s32 set_cursor_begin(u64 cursor_begin)
{
  _tm_cursor_begin = cursor_begin;
  return fill_buffer();
}

s32 insert_text(u8* text, u64 size, u64 cursor_begin)
{
  u32 block_position;
  ho_block* block = get_initial_block_at_cursor(&block_position, cursor_begin);

  if (!insert_text_in_block(block, text, block_position, size, true))
  {
    _tm_text_size += size;
    return fill_buffer();
  }
  else
    return -1;
}

s32 delete_text(u64 size, u64 cursor_begin)
{
  u32 block_position;
  ho_block* block = get_initial_block_at_cursor(&block_position, cursor_begin);

  if (cursor_begin + size > _tm_text_size)
	  error_fatal("delete_text() error: cursor_begin + size > _tm_text_size\n", 0);

  if (!delete_text_in_block(block, block_position, size, true))
  {
    _tm_text_size -= size;
    return fill_buffer();
  }
  else
    return -1;
}

s32 move_block_data(ho_block* block, u32 initial_block_position, u64 size, u8* memory_position)
{
  u32 i, current_initial_block_position = initial_block_position;
  u64 remaining_to_move = size;
  u8* current_mem_position = memory_position;
  ho_block_container* current_block_container = block->container;
  ho_block* current_block;

  i = block->position_in_container;

  while (current_block_container != null)
  {
    for (; i<current_block_container->num_blocks_in_container; ++i)
    {
      current_block = &current_block_container->blocks[i];

      if (remaining_to_move > (current_block->occupied - current_initial_block_position))
      {
        u32 move_size = current_block->occupied - current_initial_block_position;
        copy_string(current_mem_position, current_block->block_data.data + current_initial_block_position, move_size);
        current_mem_position += move_size;
        remaining_to_move -= move_size;
        current_initial_block_position = 0;
      }
      else
      {
        copy_string(current_mem_position, current_block->block_data.data + current_initial_block_position, remaining_to_move);
        return 0;
      }
    }

    i = 0;
    current_block_container = current_block_container->next;
  }

  error_fatal("move_block_data error: block overflow.", 0);
  return -1;
}

s32 fill_buffer()
{
  u32 block_position;
  ho_block* block = get_initial_block_at_cursor(&block_position, _tm_cursor_begin);

  if (block != null)
  {
    if (_tm_cursor_begin + _tm_buffer_size > _tm_text_size)
    {
      error_warning("Warning: Buffer is outside text bounds.\n");
      _tm_valid_bytes = _tm_text_size - _tm_cursor_begin;
      return move_block_data(block, block_position, _tm_text_size - _tm_cursor_begin, _tm_buffer);
    }
    else
    {
      _tm_valid_bytes = _tm_buffer_size;
      return move_block_data(block, block_position, _tm_buffer_size, _tm_buffer);
    }
  }
  else
  {
    return -1;
  }
}

ho_block* get_initial_block_at_cursor(u32* block_position, u64 cursor_begin)
{
  u64 cursor_position = 0, i;
  ho_block_container* current_block_container = main_text.block_container;
  ho_block* block_aux;
  ho_block* last_block;

  if (current_block_container == null)
  {
    error_fatal("get_initial_block_at_cursor_begin: first block container is nil.\n", 0);
    return null;
  }
  else if (current_block_container->num_blocks_in_container <= 0)
  {
    error_fatal("get_initial_block_at_cursor_begin: no blocks.\n", 0);
    return null;
  }

  while (current_block_container != null)
  {
    u32 container_ocuppied = current_block_container->total_occupied;

    if (cursor_position + container_ocuppied > cursor_begin)
      break;

    cursor_position += current_block_container->total_occupied;
    current_block_container = current_block_container->next;
  }

  for (i=0; i<current_block_container->num_blocks_in_container; ++i)
  {
    last_block = &current_block_container->blocks[i];
    u32 b_occupied = current_block_container->blocks[i].occupied;
    if (cursor_position + b_occupied > cursor_begin)
    {
      *block_position = cursor_begin - cursor_position;
      return last_block;
    }

    cursor_position += current_block_container->blocks[i].occupied;
  }

  *block_position = cursor_begin - cursor_position;
  return last_block;
}

s32 refresh_buffer()
{
  return fill_buffer();
}

void check_text()
{
  if (check_main_text(_tm_text_size))
  {
    error_warning("check_text() log: Error detected. Check console.\n");
  }
  else
  {
    log_success("check_text() log: No errors detected.\n");
  }
}

void check_arenas()
{
  if (check_main_arena_manager())
  {
    error_warning("check_arenas() log: Error detected. Check console.\n");
  }
  else
  {
    log_success("check_arenas() log: No errors detected.\n");
  }
}
