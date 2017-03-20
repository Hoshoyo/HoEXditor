#include "text_manager.h"
#include "text_events.h"
#include "memory.h"
#include "util.h"

/* BLOCK TEXT GLOBAL INFO */
internal u8* _tm_block_buffer[MAX_FILES_OPEN] = {null};                             // VIRTUAL
internal u64 _tm_block_buffer_size[MAX_FILES_OPEN];                                 // VIRTUAL
internal u64 _tm_block_cursor_begin[MAX_FILES_OPEN];                                // VIRTUAL
internal line_break_types _tm_block_cursor_line_number_reference[MAX_FILES_OPEN];   // VIRTUAL
internal u64 _tm_block_text_size[MAX_FILES_OPEN];                                   // REAL
internal u64 _tm_block_valid_bytes[MAX_FILES_OPEN];                                 // VIRTUAL
internal u8* _tm_block_file_name[MAX_FILES_OPEN] = {0};                             // REAL
/* *********************** */

/* CONTIGUOUS TEXT GLOBAL INFO */
internal u8* _tm_contiguous_real_buffer[MAX_CONTIGUOUS_TEXT_OPEN] = {null};         // REAL - This is the REAL buffer! It is allocated.
internal u64 _tm_contiguous_real_buffer_size[MAX_CONTIGUOUS_TEXT_OPEN];             // REAL
internal u8* _tm_contiguous_buffer[MAX_CONTIGUOUS_TEXT_OPEN] = {null};              // VIRTUAL - This is a FAKE buffer! It points to somewhere inside a real buffer.
internal u64 _tm_contiguous_buffer_size[MAX_CONTIGUOUS_TEXT_OPEN];                  // VIRTUAL
internal u64 _tm_contiguous_text_size[MAX_CONTIGUOUS_TEXT_OPEN];                    // REAL
internal u64 _tm_contiguous_cursor_begin[MAX_CONTIGUOUS_TEXT_OPEN];                 // VIRTUAL
/* *************************** */

// consts
internal char lf_pattern[] = "\n";
internal char crlf_pattern[] = "\r\n";
internal s32 lf_pattern_length = 1;
internal s32 crlf_pattern_length = 2;

s32 create_tid(text_id* tid, bool is_block_text)
{
  if (init_text_backbone(tid, is_block_text) < 0)
    return -1;

  if (configure_text_events(*tid) < 0)
    return -1;

  if (is_block_text)
  {
    _tm_block_buffer[tid->id] = null;
    _tm_block_buffer_size[tid->id] = -1;
    _tm_block_cursor_begin[tid->id] = -1;
    _tm_block_cursor_line_number_reference[tid->id].lf = -1;
    _tm_block_cursor_line_number_reference[tid->id].crlf = -1;
    _tm_block_cursor_line_number_reference[tid->id].cr = -1;
    _tm_block_text_size[tid->id] = -1;
    _tm_block_valid_bytes[tid->id] = -1;
    _tm_block_file_name[tid->id] = null;
  }
  else
  {
    _tm_contiguous_real_buffer[tid->id] = null;
    _tm_contiguous_real_buffer_size[tid->id] = -1;
    _tm_contiguous_buffer[tid->id] = null;
    _tm_contiguous_buffer_size[tid->id] = -1;
    _tm_contiguous_text_size[tid->id] = -1;
    _tm_contiguous_cursor_begin[tid->id] = -1;
  }

  return 0;
}

s32 load_file(text_id tid, u8* filename)
{
  if (filename == null || !tid.is_block_text)
    return -1;

  s64 size;
  store_file_name(tid, filename);
  u8* filedata = read_entire_file(filename, &size);

  u32 block_fill_value = (u32)(BLOCK_FILL_RATIO * BLOCK_SIZE);

  if (block_fill_value <= 0)
    block_fill_value = 1;

  if (size > 0)
  {
    _tm_block_text_size[tid.id] = size;
    fill_blocks_with_text(tid, filedata, size, block_fill_value);
  }
  else
  {
    _tm_block_text_size[tid.id] = 0;
  }

  return 0;
}

s32 create_real_buffer(text_id tid, u64 size)
{
  if (tid.is_block_text)
    return -1;

  if (_tm_contiguous_real_buffer[tid.id] == null)
  {
    _tm_contiguous_real_buffer[tid.id] = halloc(sizeof(u8) * size);
  }
  else if (_tm_contiguous_real_buffer_size[tid.id] != size)
  {
    hfree(_tm_contiguous_buffer[tid.id]);
    _tm_contiguous_real_buffer[tid.id] = halloc(sizeof(u8) * size);
  }

  _tm_contiguous_real_buffer_size[tid.id] = size;
  _tm_contiguous_text_size[tid.id] = 0;

  return 0;
}

s32 configure_text_events(text_id tid)
{
  // text events - tests.
  if (init_text_events(tid) < 0)
    return -1;

  u32 ak[2];
  ak[0] = 17;	// ctrl
  ak[1] = 90; // z
  update_action_command(tid, HO_UNDO, 2, ak);	// add ctrl+z command
  ak[0] = 17;	// ctrl
  ak[1] = 89; // y
  update_action_command(tid, HO_REDO, 2, ak);	// add ctrl+y command
  ak[0] = 17;	// ctrl
  ak[1] = 86; // v
  update_action_command(tid, HO_PASTE, 2, ak);	// add ctrl+v command
  ak[0] = 17;	// ctrl
  ak[1] = 67; // c
  update_action_command(tid, HO_COPY, 2, ak);	// add ctrl+c command
  ak[0] = 17;	// ctrl
  ak[1] = 70; // f
  update_action_command(tid, HO_SEARCH, 2, ak);	// add ctrl+c command

  return 0;
}

void store_file_name(text_id tid, u8* filename)
{
  if (!tid.is_block_text)
    return;

  u32 i = 0;
  u32 file_name_size = 0;
  u8* file_name_pos = filename;

  while(filename[i] != null)
  {
    ++file_name_size;

    if (filename[i] == '/' || filename[i] == '\\')
    {
      file_name_size = 0;
      file_name_pos = filename + i + 1;
    }

    ++i;
  }

  ++file_name_size;

  _tm_block_file_name[tid.id] = halloc(file_name_size * sizeof(u8));
  copy_string(_tm_block_file_name[tid.id], file_name_pos, file_name_size);
}

s32 finalize_tid(text_id tid)
{
  if (tid.is_block_text)
  {
    hfree(_tm_block_buffer[tid.id]);
    _tm_block_buffer[tid.id] = null;
    _tm_block_cursor_begin[tid.id] = -1;
    _tm_block_buffer_size[tid.id] = -1;
    _tm_block_text_size[tid.id] = -1;
    _tm_block_valid_bytes[tid.id] = -1;
    hfree(_tm_block_file_name[tid.id]);
    _tm_block_file_name[tid.id] = null;
  }
  else
  {
    hfree(_tm_contiguous_real_buffer[tid.id]);
    _tm_contiguous_real_buffer[tid.id] = null;
    _tm_contiguous_real_buffer_size[tid.id] = -1;
    _tm_contiguous_buffer[tid.id] = null;
    _tm_contiguous_buffer_size[tid.id] = -1;
    _tm_contiguous_text_size[tid.id] = -1;
  }

  finalize_text_events(tid);

  return destroy_text_backbone(tid);
}

void fill_blocks_with_text(text_id tid, u8* data, s64 data_size, u32 block_fill_value)
{
  if (!tid.is_block_text)
    return;

  u64 data_remaining = data_size;
  u8* data_pointer = data;

  ho_block* last_block = &(_t_texts[tid.id]->block_container->blocks[0]);

  // first block must be populated individually.
  if (data_remaining > block_fill_value)
  {
    insert_text_in_block(tid, last_block, data_pointer, 0, block_fill_value, false);
    data_pointer += block_fill_value;
    data_remaining -= block_fill_value;
  }
  else
  {
    insert_text_in_block(tid, last_block, data_pointer, 0, data_remaining, false);
    data_pointer += data_remaining;
    data_remaining -= data_remaining;
  }

  // populates all other blocks, except the last one.
  while (data_remaining >= block_fill_value)
  {
    last_block = append_block(tid, *last_block);
    insert_text_in_block(tid, last_block, data_pointer, 0, block_fill_value, false);
    data_pointer += block_fill_value;
    data_remaining -= block_fill_value;
  }

  if (data_remaining > 0)
  {
    // populates last block
    last_block = append_block(tid, *last_block);
    insert_text_in_block(tid, last_block, data_pointer, 0, data_remaining, false);
    data_pointer += data_remaining;
    data_remaining -= data_remaining;
  }
}

u8* get_text_buffer(text_id tid, u64 size, u64 cursor_begin)
{
  if (tid.is_block_text)
  {
    u32 ret;

    if (_tm_block_buffer[tid.id] == null)
    {
      _tm_block_buffer[tid.id] = halloc(sizeof(u8) * size);
    }
    else if (_tm_block_buffer_size[tid.id] != size)
    {
      hfree(_tm_block_buffer[tid.id]);
      _tm_block_buffer[tid.id] = halloc(sizeof(u8) * size);
    }

    _tm_block_buffer_size[tid.id] = size;
    _tm_block_cursor_begin[tid.id] = cursor_begin;

    ret = fill_buffer(tid);

    if (!ret)
      return _tm_block_buffer[tid.id];
    else
    {
      hfree(_tm_block_buffer[tid.id]);
      return null;
    }
  }
  else
  {
    _tm_contiguous_buffer[tid.id] = _tm_contiguous_real_buffer[tid.id] + cursor_begin;  // points to real_buffer[cursor_begin]
    _tm_contiguous_buffer_size[tid.id] = size;
    _tm_contiguous_cursor_begin[tid.id] = cursor_begin;

    return _tm_contiguous_buffer[tid.id];
  }
}

u64 get_tid_valid_bytes(text_id tid)
{
  if (tid.is_block_text)
    return _tm_block_valid_bytes[tid.id];
  else
    return _tm_contiguous_text_size[tid.id] - _tm_contiguous_cursor_begin[tid.id];
}

u64 get_tid_text_size(text_id tid)
{
  if (tid.is_block_text)
    return _tm_block_text_size[tid.id];
  else
    return _tm_contiguous_text_size[tid.id];
}

u8* get_tid_file_name(text_id tid)
{
  if (tid.is_block_text)
    return _tm_block_file_name[tid.id];
  else
    return null;
}

u8* get_text_as_contiguous_memory(text_id tid, u64* text_size)
{
  if (tid.is_block_text)
  {
    *text_size = 0;
    ho_block_container* current_block_container = _t_texts[tid.id]->block_container;

    while (current_block_container != null)
    {
      *text_size += current_block_container->total_occupied;
      current_block_container = current_block_container->next;
    }

    u8* text = halloc(sizeof(u8) * (*text_size));
    u64 already_copied = 0;

    current_block_container = _t_texts[tid.id]->block_container;

    while (current_block_container != null)
    {
      for (u32 i=0; i<current_block_container->num_blocks_in_container; ++i)
      {
        ho_block block = current_block_container->blocks[i];
        copy_string(text + already_copied, block.block_data.data, block.occupied);
        already_copied += block.occupied;
      }

      current_block_container = current_block_container->next;
    }

    return text;
  }
  else
  {
    u8* text = halloc(sizeof(u8) * _tm_contiguous_text_size[tid.id]);
    copy_string(text, _tm_contiguous_real_buffer[tid.id], _tm_contiguous_text_size[tid.id] * sizeof(u8));
    return text;
  }
}

s32 set_cursor_begin(text_id tid, u64 cursor_begin)
{
  if (tid.is_block_text)
  {
    _tm_block_cursor_begin[tid.id] = cursor_begin;
    return fill_buffer(tid);
  }
  else
  {
    _tm_contiguous_cursor_begin[tid.id] = cursor_begin;
    _tm_contiguous_buffer[tid.id] = _tm_contiguous_real_buffer[tid.id] + cursor_begin;
    return 0;
  }
}

s32 insert_text(text_id tid, u8* text, u64 size, u64 cursor_begin)
{
  if (tid.is_block_text)
  {
    u32 block_position;
    ho_block* block = get_initial_block_at_cursor(tid, &block_position, cursor_begin);

    if (!insert_text_in_block(tid, block, text, block_position, size, true))
    {
      _tm_block_text_size[tid.id] += size;
      return fill_buffer(tid);
    }
    else
      return -1;
  }
  else
  {
    copy_string(_tm_contiguous_real_buffer[tid.id] + cursor_begin + size,
      _tm_contiguous_real_buffer[tid.id] + cursor_begin,
      _tm_contiguous_text_size[tid.id] - cursor_begin);
    copy_string(_tm_contiguous_real_buffer[tid.id] + cursor_begin, text, size);

    _tm_contiguous_text_size[tid.id] += size;
    return 0;
  }
}

s32 delete_text(text_id tid, u8* text, u64 size, u64 cursor_begin)
{
  if (tid.is_block_text)
  {
    u32 block_position;
    ho_block* block = get_initial_block_at_cursor(tid, &block_position, cursor_begin);

    if (text != null)
      move_block_data(block, block_position, size, text);

    if (cursor_begin + size > _tm_block_text_size[tid.id])
  	  error_fatal("delete_text() error: cursor_begin + size > _tm_block_text_size\n", 0);

    if (!delete_text_in_block(tid, block, block_position, size, true))
    {
      _tm_block_text_size[tid.id] -= size;
      return fill_buffer(tid);
    }
    else
      return -1;
  }
  else
  {
    copy_string(_tm_contiguous_real_buffer[tid.id] + cursor_begin,
      _tm_contiguous_real_buffer[tid.id] + cursor_begin + size,
      _tm_contiguous_text_size[tid.id] - (cursor_begin + size));

    _tm_contiguous_text_size[tid.id] -= size;
    return 0;
  }
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

bool test_if_pattern_match(ho_block* block, u32 block_position, u8* pattern, u64 pattern_length)
{
  ho_block_container* current_block_container = block->container;
  s32 current_block_position = block->position_in_container;
  ho_block* current_block = block;
  s32 pattern_position = 0;

  while (current_block_container != null)
  {
    for (; current_block_position < current_block_container->num_blocks_in_container; ++current_block_position)
    {
      current_block = &current_block_container->blocks[current_block_position];
      for (; block_position<current_block->occupied; ++block_position)
      {
        if (current_block->block_data.data[block_position] != pattern[pattern_position])
          return false;

        ++pattern_position;
        if (pattern_position == pattern_length)
          return true;
      }

      block_position = 0;
    }

    current_block_position = 0;
    current_block_container = current_block_container->next;
  }

  error_fatal("test_if_pattern_match error(): block overflow.\n", 0);
  return false;
}

bool test_if_pattern_match_backwards(ho_block* block, u32 block_position, u8* pattern, u64 pattern_length)
{
  ho_block_container* current_block_container = block->container;
  s32 current_block_position = block->position_in_container;
  ho_block* current_block = block;
  s32 pattern_position = pattern_length - 1;
  bool first_time = true;

  while (current_block_container != null)
  {
    for (; current_block_position >= 0; --current_block_position)
    {
      current_block = &current_block_container->blocks[current_block_position];

      if (!first_time)
        block_position = current_block->occupied;

      for (; block_position >= 0; --block_position)
      {
        if (current_block->block_data.data[block_position] != pattern[pattern_position])
          return false;

        if (pattern_position == 0)
          return true;

        --pattern_position;
      }

      first_time = false;
    }

    current_block_container = current_block_container->previous;

    if (current_block_container != null)
      current_block_position = current_block_container->num_blocks_in_container - 1;
    else
      return false;
  }

  error_fatal("test_if_pattern_match error(): block overflow.\n", 0);
  return false;
}

s64 get_number_of_pattern_occurrences(text_id tid, u64 cursor_begin, u64 cursor_end, u8* pattern, u64 pattern_length)
{
  if (!tid.is_block_text)
    return -1;

  s32 block_position;
  ho_block* current_block = get_initial_block_at_cursor(tid, &block_position, cursor_begin);
  ho_block_container* current_block_container = current_block->container;
  s64 current_cursor_position = cursor_begin;
  s32 current_block_position = current_block->position_in_container;
  s64 pattern_occurrences = 0;

  if (pattern_length == 0 || cursor_begin < 0 || cursor_end >= _tm_block_text_size[tid.id] || pattern_length > (cursor_end - cursor_begin))
    return 0;

  while (current_cursor_position <= (cursor_end - pattern_length + 1))
  {
    for (; current_block_position < current_block_container->num_blocks_in_container; ++current_block_position)
    {
      current_block = &current_block_container->blocks[current_block_position];
      for (; block_position<current_block->occupied; ++block_position)
      {
        if (current_block->block_data.data[block_position] == pattern[0])
          if (test_if_pattern_match(current_block, block_position, pattern, pattern_length))
            ++pattern_occurrences;

        if (current_cursor_position == (cursor_end - pattern_length + 1))
          return pattern_occurrences;

        ++current_cursor_position;
      }
      block_position = 0;
    }
    current_block_position = 0;
    current_block_container = current_block_container->next;
  }

  return pattern_occurrences;
}

s64 find_next_pattern_forward(text_id tid, u64 cursor_begin, u64 cursor_end, u8* pattern, u64 pattern_length)
{
  if (!tid.is_block_text)
    return -1;

  s32 block_position;
  ho_block* current_block = get_initial_block_at_cursor(tid, &block_position, cursor_begin);
  ho_block_container* current_block_container = current_block->container;
  s64 current_cursor_position = cursor_begin;
  s32 current_block_position = current_block->position_in_container;

  if (pattern_length == 0 || cursor_begin < 0 || cursor_end >= _tm_block_text_size[tid.id] || pattern_length > (cursor_end - cursor_begin + 1))
    return -1;

  while (current_cursor_position <= (cursor_end - pattern_length + 1))
  {
    for (; current_block_position < current_block_container->num_blocks_in_container; ++current_block_position)
    {
      current_block = &current_block_container->blocks[current_block_position];
      for (; block_position<current_block->occupied; ++block_position)
      {
        if (current_block->block_data.data[block_position] == pattern[0])
          if (test_if_pattern_match(current_block, block_position, pattern, pattern_length))
              return current_cursor_position;

        if (current_cursor_position == (cursor_end - pattern_length + 1))
          return -1;

        ++current_cursor_position;
      }
      block_position = 0;
    }
    current_block_position = 0;
    current_block_container = current_block_container->next;
  }

  return -1;
}

s64 find_next_pattern_backwards(text_id tid, u64 cursor_begin, u64 cursor_end, u8* pattern, u64 pattern_length)
{
  if (!tid.is_block_text)
    return -1;

  s32 block_position;
  ho_block* current_block = get_initial_block_at_cursor(tid, &block_position, cursor_begin);
  ho_block_container* current_block_container = current_block->container;
  s64 current_cursor_position = cursor_begin;
  s32 current_block_position = current_block->position_in_container;
  bool first_time = true;

  if (pattern_length == 0 || cursor_end < 0 || cursor_begin >= _tm_block_text_size[tid.id] || pattern_length > (cursor_begin - cursor_end))
    return -1;

  while (current_cursor_position >= (cursor_end + pattern_length))
  {
    for (; current_block_position >= 0; --current_block_position)
    {
      current_block = &current_block_container->blocks[current_block_position];

      if (!first_time)
        block_position = current_block_container->blocks[current_block_position].occupied - 1;

      for (; block_position >= 0; --block_position)
      {
        if (current_block->block_data.data[block_position] == pattern[pattern_length - 1])
          if (test_if_pattern_match_backwards(current_block, block_position, pattern, pattern_length))
              return current_cursor_position;

        if (current_cursor_position == (cursor_end + pattern_length))
          return -1;

        --current_cursor_position;
      }

      first_time = false;
    }
    current_block_container = current_block_container->previous;

    if (current_block_container != null)
      current_block_position = current_block_container->num_blocks_in_container - 1;
    else
      return -1;
  }

  return -1;
}

void refresh_cursor_info_reference(text_id tid)
{
  if (!tid.is_block_text)
    return;

  u64 cursor_position = _tm_block_cursor_begin[tid.id];

  _tm_block_cursor_line_number_reference[tid.id].lf = get_number_of_pattern_occurrences(tid, 0, cursor_position, lf_pattern, lf_pattern_length);
  _tm_block_cursor_line_number_reference[tid.id].cr = -1;
  _tm_block_cursor_line_number_reference[tid.id].crlf = -1;
}

s32 fill_buffer(text_id tid)
{
  if (!tid.is_block_text)
    return -1;

  u32 block_position;
  ho_block* block = get_initial_block_at_cursor(tid, &block_position, _tm_block_cursor_begin[tid.id]);

  if (block != null)
  {
    if (_tm_block_cursor_begin[tid.id] + _tm_block_buffer_size[tid.id] > _tm_block_text_size[tid.id])
    {
      error_warning("Warning: Buffer is outside text bounds.\n");
      _tm_block_valid_bytes[tid.id] = _tm_block_text_size[tid.id] - _tm_block_cursor_begin[tid.id];
      if (move_block_data(block, block_position, _tm_block_text_size[tid.id] - _tm_block_cursor_begin[tid.id], _tm_block_buffer[tid.id]) < 0)
        return -1;
    }
    else
    {
      _tm_block_valid_bytes[tid.id] = _tm_block_buffer_size[tid.id];
      if (move_block_data(block, block_position, _tm_block_buffer_size[tid.id], _tm_block_buffer[tid.id]) < 0)
        return -1;
    }

    refresh_cursor_info_reference(tid);
    return 0;
  }
  else
  {
    return -1;
  }
}

ho_block* get_initial_block_at_cursor(text_id tid, u32* block_position, u64 cursor_begin)
{
  if (!tid.is_block_text)
    return null;

  u64 cursor_position = 0, i;
  ho_block_container* current_block_container = _t_texts[tid.id]->block_container;
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

    if (current_block_container->next != null)
      current_block_container = current_block_container->next;
    else
    {
      // End of Text reached
      // Test if only 1 cursor position is ahead end of text. If yes, then return the position after end of text. If not, error.
      if (cursor_begin == cursor_position)
      {
        *block_position = current_block_container->blocks[current_block_container->num_blocks_in_container - 1].occupied;
        return &current_block_container->blocks[current_block_container->num_blocks_in_container - 1];
      }
      else
        error_fatal("get_initial_block_at_cursor() error: can't reach cursor.\n", 0);
    }
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

cursor_info get_cursor_info(text_id tid, u64 cursor_position)
{
  cursor_info cinfo;

  if (tid.is_block_text)
  {
    cinfo.line_number.lf = _tm_block_cursor_line_number_reference[tid.id].lf;

    if (cursor_position > _tm_block_cursor_begin[tid.id])
    {
      s64 pattern_occurrences = get_number_of_pattern_occurrences(tid, _tm_block_cursor_begin[tid.id], cursor_position, lf_pattern, lf_pattern_length);
      cinfo.line_number.lf += pattern_occurrences;
    }
    else if (cursor_position < _tm_block_cursor_begin[tid.id])
    {
      s64 pattern_occurrences = get_number_of_pattern_occurrences(tid, cursor_position, _tm_block_cursor_begin[tid.id], lf_pattern, lf_pattern_length);
      cinfo.line_number.lf -= pattern_occurrences;
    }

    if (cursor_position != 0)
      cinfo.previous_line_break.lf = find_next_pattern_backwards(tid, cursor_position - 1, 0, lf_pattern, lf_pattern_length);
    else
      cinfo.previous_line_break.lf = -1;

    if (cursor_position != _tm_block_text_size[tid.id])
      cinfo.next_line_break.lf = find_next_pattern_forward(tid, cursor_position, _tm_block_text_size[tid.id] - 1, lf_pattern, lf_pattern_length);
    else
      cinfo.next_line_break.lf = -1;

    return cinfo;
  }
  else
  {
    // @TODO: get_cursor_info contiguous mode
    cinfo.line_number.lf = 0;
    cinfo.previous_line_break.lf = 0;
    cinfo.next_line_break.lf = 0;
    return cinfo;
  }
}

s32 refresh_buffer(text_id tid)
{
  if (!tid.is_block_text)
    return -1;

  return fill_buffer(tid);
}

void check_text(text_id tid)
{
  if (!tid.is_block_text)
    return;

  if (check_text_backbone(tid, _tm_block_text_size[tid.id]))
    error_warning("check_text() log: Error detected. Check console.\n");
  else
    log_success("check_text() log: No errors detected.\n");
}

void check_arenas(text_id tid)
{
  if (!tid.is_block_text)
    return;

  if (check_arena_backbone(tid))
    error_warning("check_arenas() log: Error detected. Check console.\n");
  else
    log_success("check_arenas() log: No errors detected.\n");
}
