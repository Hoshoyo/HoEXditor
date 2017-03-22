#include "text_events.h"
#include "input.h"
#include "text_manager.h"
#include "memory.h"
#include "editor.h"
#include "util.h"
#include "os_dependent.h"

#define MOD(n) (n) > 0 ? (n) : -(n)

ho_text_events* _te_block_text_events[MAX_FILES_OPEN];
ho_text_events* _te_contiguous_text_events[MAX_CONTIGUOUS_TEXT_OPEN];

s32 init_text_events(text_id tid)
{
  ho_text_events* new_text_event = halloc(sizeof(ho_text_events));

  new_text_event->num_undo_items = 0;
  new_text_event->num_redo_items = 0;
  new_text_event->undo_stack_begin = 0;
  new_text_event->redo_stack_begin = 0;
  new_text_event->max_undo_items = MAX_UNDO_MEM;
  new_text_event->max_redo_items = MAX_REDO_MEM;
  new_text_event->num_action_commands = 0;

  if (tid.is_block_text)
    _te_block_text_events[tid.id] = new_text_event;
  else
    _te_contiguous_text_events[tid.id] = new_text_event;

  return 0;
}

s32 finalize_text_events(text_id tid)
{
  clear_events(tid);

  if (tid.is_block_text)
  {
    hfree(_te_block_text_events[tid.id]);
    _te_block_text_events[tid.id] = null;
  }
  else
  {
    hfree(_te_contiguous_text_events[tid.id]);
    _te_contiguous_text_events[tid.id] = null;
  }

  return 0;
}

s32 save_file(text_id tid, u8* filename)
{
  u64 size;
	u8* text = get_text_as_contiguous_memory(tid, &size);

	s32 written_bytes = write_file(filename, text, size);
  hfree(text);

  if (written_bytes == size)
    return 0;
  else
    return -1;
}

void keyboard_call_events(Editor_State* es)
{
  u32 i, j;
  ho_text_events** _te_text_events;

  if (es->main_buffer_tid.is_block_text)
    _te_text_events = _te_block_text_events;
  else
    _te_text_events = _te_contiguous_text_events;

  for (i=0; i<_te_text_events[es->main_buffer_tid.id]->num_action_commands; ++i)
  {
    // testar se todas keys foram pressionadas!
    for (j=0; j<_te_text_events[es->main_buffer_tid.id]->action_commands[i].num_associated_keys; ++j)
      if (!keyboard_state.key[_te_text_events[es->main_buffer_tid.id]->action_commands[i].associated_keys[j]])
        break;

    if (j == _te_text_events[es->main_buffer_tid.id]->action_commands[i].num_associated_keys)
      execute_action_command(es, _te_text_events[es->main_buffer_tid.id]->action_commands[i].type);
  }
}

void execute_action_command(Editor_State* es, enum ho_action_command_type type)
{
  switch (type)
  {
    case HO_UNDO:
    {
      do_undo(es);
    } break;
    case HO_REDO:
    {
      do_redo(es);
    } break;
    case HO_COPY:
    {
      if (es->selecting)
      {
        s64 bytes_to_copy = MOD(es->cursor_info.selection_offset - es->cursor_info.cursor_offset);
        s64 cursor_begin = MIN(es->cursor_info.selection_offset, es->cursor_info.cursor_offset);

        if (bytes_to_copy > 0)
        {
          open_clipboard();
          u8* text_to_copy = halloc(bytes_to_copy * sizeof(u8));
          copy_text_to_contiguous_memory_area(es->main_buffer_tid, cursor_begin, bytes_to_copy, text_to_copy);
          set_clipboard_content(text_to_copy, bytes_to_copy);
          close_clipboard();
        }
      }
    } break;
    case HO_CUT:
    {

    } break;
    case HO_PASTE:
    {
      u8* text;
      u8* new_text;

      open_clipboard();
      get_clipboard_content(&text);

      u64 text_size = hstrlen(text);
      new_text = halloc(text_size * sizeof(u8));
      copy_string(new_text, text, text_size);

      insert_text(es->main_buffer_tid, new_text, text_size, es->cursor_info.cursor_offset);
      add_undo_item(es->main_buffer_tid, HO_INSERT_TEXT, new_text, text_size, es->cursor_info.cursor_offset);
      es->cursor_info.cursor_offset += text_size;

      close_clipboard();
    } break;
    case HO_SEARCH:
    {
      // call UI
      print("HO_SEARCH called.\n");
    } break;
    case HO_REPLACE:
    {
      // call UI
      print("HO_REPLACE called\n");
    } break;
    case HO_SAVE:
    {
      save_file(es->main_buffer_tid, get_tid_file_name(es->main_buffer_tid));
      log_success("File Saved Successfully.\n");
    } break;
  }
}

void handle_char_press(Editor_State* es, u8 key)
{
  // if there's a selection, delete it
  if (es->selecting)
  {
    s64 bytes_to_delete = MOD(es->cursor_info.selection_offset - es->cursor_info.cursor_offset);
    s64 cursor_begin = MIN(es->cursor_info.selection_offset, es->cursor_info.cursor_offset);
    s64 move_cursor = (es->cursor_info.selection_offset > es->cursor_info.cursor_offset) ? 0 : bytes_to_delete;

    u8* deleted_text = halloc(bytes_to_delete * sizeof(u8));

    delete_text(es->main_buffer_tid, deleted_text, bytes_to_delete * sizeof(u8), cursor_begin);
    add_undo_item(es->main_buffer_tid, HO_DELETE_TEXT, deleted_text, bytes_to_delete * sizeof(u8), cursor_begin);

    es->cursor_info.cursor_offset -= move_cursor;
  }

  switch (key)
  {
    case CARRIAGE_RETURN_KEY:
    {
      // temporary - this is OS dependent
      u8* inserted_text = halloc(sizeof(u8));
      inserted_text[0] = LINE_FEED_KEY;

      insert_text(es->main_buffer_tid, inserted_text, 1, es->cursor_info.cursor_offset);
      add_undo_item(es->main_buffer_tid, HO_INSERT_TEXT, inserted_text, 1 * sizeof(u8), es->cursor_info.cursor_offset);

      es->cursor_info.cursor_offset += 1;
    } break;
    case BACKSPACE_KEY:
    {
      if (!es->selecting && es->cursor_info.cursor_offset > 0)
      {
      	u8* deleted_text = halloc(sizeof(u8));

      	delete_text(es->main_buffer_tid, deleted_text, sizeof(u8), es->cursor_info.cursor_offset - 1);
      	add_undo_item(es->main_buffer_tid, HO_DELETE_TEXT, deleted_text, sizeof(u8), es->cursor_info.cursor_offset - 1);

      	es->cursor_info.cursor_offset -= 1;
      }
    } break;
    default:
    {
      u8* inserted_text = halloc(sizeof(u8));
      *inserted_text = key;

      insert_text(es->main_buffer_tid, inserted_text, 1, es->cursor_info.cursor_offset);
      add_undo_item(es->main_buffer_tid, HO_INSERT_TEXT, inserted_text, sizeof(u8), es->cursor_info.cursor_offset);

      es->cursor_info.cursor_offset += 1;
    } break;
  }

	check_text(es->main_buffer_tid);
	check_arenas(es->main_buffer_tid);
}

ho_search_result* search_word(text_id tid, u64 cursor_begin, u64 cursor_end, u8* pattern, u64 pattern_length)
{
  if (tid.is_block_text)
  {
    ho_search_result* result = null;
    ho_search_result* last_result = null;
    s32 block_position;
    ho_block* current_block = get_initial_block_at_cursor(tid, &block_position, cursor_begin);
    ho_block_container* current_block_container = current_block->container;
    s64 current_cursor_position = cursor_begin;
    s32 current_block_position = current_block->position_in_container;

    if (pattern_length == 0 || (cursor_begin + pattern_length) > get_tid_text_size(tid) || pattern_length > cursor_end)
      return null;

    while (current_cursor_position <= (cursor_end - pattern_length + 1))
    {
      for (; current_block_position < current_block_container->num_blocks_in_container; ++current_block_position)
      {
        current_block = &current_block_container->blocks[current_block_position];
        for (; block_position<current_block->occupied; ++block_position)
        {
          if (current_block->block_data.data[block_position] == pattern[0])
            if (test_if_pattern_match(current_block, block_position, pattern, pattern_length))
            {
              if (last_result == null)
              {
                result = halloc(sizeof(ho_search_result));
                last_result = result;
                result->next = null;
                result->cursor_position = current_cursor_position;
              }
              else
              {
                last_result->next = halloc(sizeof(ho_search_result));
                last_result->next->cursor_position = current_cursor_position;
                last_result->next->next = null;
                last_result = last_result->next;
              }
            }

          if (current_cursor_position == (cursor_end - pattern_length + 1))
            return result;

          ++current_cursor_position;
        }
        block_position = 0;
      }
      current_block_position = 0;
      current_block_container = current_block_container->next;
    }

    return result;
  }
  else
  {
    // @TODO: Search Word Contiguous Text
    return null;
  }
}

void add_undo_item(text_id tid, enum ho_action_type type, u8* text, u64 text_size, u64 cursor_position)
{
  ho_aiv_undo_redo* aiv = halloc(sizeof(ho_aiv_undo_redo));
  aiv->text = text;
  aiv->text_size = text_size;
  aiv->cursor_position = cursor_position;

  ho_action_item action_item;
  action_item.type = type;
  action_item.value = aiv;
  push_stack_item(tid, HO_UNDO_STACK, action_item);

  // empty redo stack.
  empty_stack(tid, HO_REDO_STACK);
}

void add_redo_item(text_id tid, enum ho_action_type type, u8* text, u64 text_size, u64 cursor_position)
{
  ho_aiv_undo_redo* aiv = halloc(sizeof(ho_aiv_undo_redo));
  aiv->text = text;
  aiv->text_size = text_size;
  aiv->cursor_position = cursor_position;

  ho_action_item action_item;
  action_item.type = type;
  action_item.value = aiv;
  push_stack_item(tid, HO_REDO_STACK, action_item);
}

void update_action_command(text_id tid, enum ho_action_command_type type, u32 num_associated_keys, u32* associated_keys)
{
  ho_text_events** _te_text_events;

  if (tid.is_block_text)
    _te_text_events = _te_block_text_events;
  else
    _te_text_events = _te_contiguous_text_events;

  for (u32 i=0; i<_te_text_events[tid.id]->num_action_commands; ++i)
    if (type == _te_text_events[tid.id]->action_commands[i].type)
    {
      _te_text_events[tid.id]->action_commands[i].num_associated_keys = num_associated_keys;
      for (u32 j=0; j<num_associated_keys; ++j)
        _te_text_events[tid.id]->action_commands[i].associated_keys[j] = associated_keys[j];
      return;
    }

  if (_te_text_events[tid.id]->num_action_commands == MAX_ACTION_COMMANDS)
  {
    error_warning("Error: Action Command could not be added. No space.\n");
    return;
  }

  ho_action_command action_command;

  action_command.type = type;
  action_command.num_associated_keys = num_associated_keys;

  for (u32 j=0; j<num_associated_keys; ++j)
    action_command.associated_keys[j] = associated_keys[j];

  _te_text_events[tid.id]->action_commands[_te_text_events[tid.id]->num_action_commands] = action_command;
  ++_te_text_events[tid.id]->num_action_commands;
}

void remove_action_command(text_id tid, enum ho_action_command_type type)
{
  // @TODO : remove_action_command
}

void fill_stack_attrbs(text_id tid, HO_EVENT_STACK stack, u32** stack_begin, u32** stack_max_items, u32** stack_num_items, ho_action_item** stack_items)
{
  ho_text_events** _te_text_events;

  if (tid.is_block_text)
    _te_text_events = _te_block_text_events;
  else
    _te_text_events = _te_contiguous_text_events;

  switch (stack)
  {
    case HO_UNDO_STACK:
      *stack_begin = &(_te_text_events[tid.id]->undo_stack_begin);
      *stack_max_items = &(_te_text_events[tid.id]->max_undo_items);
      *stack_num_items = &(_te_text_events[tid.id]->num_undo_items);
      *stack_items = _te_text_events[tid.id]->undo_items;
      break;
    case HO_REDO_STACK:
      *stack_begin = &(_te_text_events[tid.id]->redo_stack_begin);
      *stack_max_items = &(_te_text_events[tid.id]->max_redo_items);
      *stack_num_items = &(_te_text_events[tid.id]->num_redo_items);
      *stack_items = _te_text_events[tid.id]->redo_items;
      break;
  }
}

s32 push_stack_item(text_id tid, HO_EVENT_STACK stack, ho_action_item item)
{
  u32 *stack_begin, *stack_max_items, *stack_num_items;
  ho_action_item* stack_items;
  ho_action_item old_item;

  fill_stack_attrbs(tid, stack, &stack_begin, &stack_max_items, &stack_num_items, &stack_items);

  if (*stack_begin == *stack_max_items - 1)
  {
    *stack_begin = 0;
    old_item = stack_items[0];
    stack_items[0] = item;
  }
  else
  {
    ++*stack_begin;
    old_item = stack_items[*stack_begin];
    stack_items[*stack_begin] = item;
  }

  if (*stack_num_items != *stack_max_items)
    ++*stack_num_items;
  else
    free_action_item(old_item);

  return 0;
}

ho_action_item pop_stack_item(text_id tid, HO_EVENT_STACK stack)
{
  u32 *stack_begin, *stack_max_items, *stack_num_items;
  ho_action_item* stack_items;
  ho_action_item item;

  fill_stack_attrbs(tid, stack, &stack_begin, &stack_max_items, &stack_num_items, &stack_items);

  if (*stack_num_items == 0)
  {
    error_fatal("pop stack item() error: popping an empty stack.\n", 0);
  }

  if (*stack_begin == 0)
  {
    item = stack_items[0];
    *stack_begin = *stack_max_items - 1;
  }
  else
  {
    item = stack_items[*stack_begin];
    --*stack_begin;
  }

  --*stack_num_items;

  return item;
}

void print_stack(text_id tid, HO_EVENT_STACK stack)
{
  u32 *stack_begin, *stack_max_items, *stack_num_items;
  ho_action_item* stack_items;

  fill_stack_attrbs(tid, stack, &stack_begin, &stack_max_items, &stack_num_items, &stack_items);

  s32 current_array_position = *stack_begin + *stack_max_items; // + *stack_max_items is to handle negative current_array_position in loop
  u32 current_number_of_items = 0;
  u32 total_number_of_items = *stack_num_items;
  u32 max_number_of_items = *stack_max_items;
  ho_action_item current_action_item;

  print("\n--- UNDO STACK PRINT ---\n\n");
  print("Stack Number of Items: %u\n", total_number_of_items);
  print("Stack Max Number of Items: %u\n", max_number_of_items);
  print("Stack Begin Position: %u\n", current_array_position);
  print("Data: \n\n");

  while (current_number_of_items != total_number_of_items)
  {
    u32 real_array_position = current_array_position % max_number_of_items;
    current_action_item = stack_items[real_array_position];

    print("%u. Key: %d, Value: %p\n", current_number_of_items + 1, current_action_item.type, current_action_item.value);

    ++current_number_of_items;
    --current_array_position;
  }
}

void do_undo(Editor_State* es)
{
  if (is_stack_empty(es->main_buffer_tid, HO_UNDO_STACK))
    return;

  ho_action_item action_item = pop_stack_item(es->main_buffer_tid, HO_UNDO_STACK);
  push_stack_item(es->main_buffer_tid, HO_REDO_STACK, copy_action_item(action_item));

  switch (action_item.type)
  {
    case HO_INSERT_TEXT:
    {
      ho_aiv_undo_redo* aiv = (ho_aiv_undo_redo*) action_item.value;
      u32 cursor_position = aiv->cursor_position;
      u32 text_size = aiv->text_size;
      u8* text = aiv->text;
      delete_text(es->main_buffer_tid, null, text_size, cursor_position);
      es->cursor_info.cursor_offset = cursor_position;
      free_action_item(action_item);
    } break;
    case HO_DELETE_TEXT:
    {
      ho_aiv_undo_redo* aiv = (ho_aiv_undo_redo*) action_item.value;
      u32 cursor_position = aiv->cursor_position;
      u32 text_size = aiv->text_size;
      u8* text = aiv->text;
      insert_text(es->main_buffer_tid, text, text_size, cursor_position);
      es->cursor_info.cursor_offset = cursor_position + text_size;
      free_action_item(action_item);
    } break;
  }
}

void do_redo(Editor_State* es)
{
  if (is_stack_empty(es->main_buffer_tid, HO_REDO_STACK))
    return;

  ho_action_item action_item = pop_stack_item(es->main_buffer_tid, HO_REDO_STACK);
  push_stack_item(es->main_buffer_tid, HO_UNDO_STACK, copy_action_item(action_item));

  switch (action_item.type)
  {
    case HO_INSERT_TEXT:
    {
      ho_aiv_undo_redo* aiv = (ho_aiv_undo_redo*) action_item.value;
      u32 cursor_position = aiv->cursor_position;
      u32 text_size = aiv->text_size;
      u8* text = aiv->text;
      insert_text(es->main_buffer_tid, text, text_size, cursor_position);
      es->cursor_info.cursor_offset = cursor_position + text_size;
      free_action_item(action_item);
    } break;
    case HO_DELETE_TEXT:
    {
      ho_aiv_undo_redo* aiv = (ho_aiv_undo_redo*) action_item.value;
      u32 cursor_position = aiv->cursor_position;
      u32 text_size = aiv->text_size;
      u8* text = aiv->text;
      delete_text(es->main_buffer_tid, null, text_size, cursor_position);
      es->cursor_info.cursor_offset = cursor_position;
      free_action_item(action_item);
    } break;
  }
}

ho_action_item copy_action_item(ho_action_item action_item)
{
  ho_action_item ai;
  ai.type = action_item.type;

  switch (ai.type)
  {
    case HO_INSERT_TEXT:
    case HO_DELETE_TEXT:
    {
      ho_aiv_undo_redo* existing_aiv = (ho_aiv_undo_redo*)action_item.value;
      ho_aiv_undo_redo* new_aiv = halloc(sizeof(ho_aiv_undo_redo));
      new_aiv->cursor_position = existing_aiv->cursor_position;
      new_aiv->text_size = existing_aiv->text_size;
      new_aiv->text = halloc(new_aiv->text_size * sizeof(u8));
      copy_string(new_aiv->text, existing_aiv->text, new_aiv->text_size);
      ai.value = new_aiv;
      return ai;
    } break;
    default:
      return ai;
  }
}

void free_action_item(ho_action_item action_item)
{
  switch (action_item.type)
  {
    case HO_INSERT_TEXT:
    case HO_DELETE_TEXT:
    {
      ho_aiv_undo_redo* aiv = (ho_aiv_undo_redo*)action_item.value;
      hfree(aiv->text);
      hfree(aiv);
    } break;
    default:
    {

    } break;
  }
}

bool is_stack_empty(text_id tid, HO_EVENT_STACK stack)
{
  u32 *stack_begin, *stack_max_items, *stack_num_items;
  ho_action_item* stack_items;

  fill_stack_attrbs(tid, stack, &stack_begin, &stack_max_items, &stack_num_items, &stack_items);

  if (*stack_num_items == 0)
    return true;

  return false;
}

void empty_stack(text_id tid, HO_EVENT_STACK stack)
{
  u32 *stack_begin, *stack_max_items, *stack_num_items;
  ho_action_item* stack_items;

  fill_stack_attrbs(tid, stack, &stack_begin, &stack_max_items, &stack_num_items, &stack_items);

  switch (stack)
  {
    case HO_UNDO_STACK:
    case HO_REDO_STACK:
    {
      s32 current_array_position = *stack_begin + *stack_max_items; // + *stack_max_items is to handle negative current_array_position in loop
      u32 current_number_of_items = 0;
      u32 total_number_of_items = *stack_num_items;
      u32 max_number_of_items = *stack_max_items;
      ho_action_item current_action_item;

      while (current_number_of_items != total_number_of_items)
      {
        u32 real_array_position = current_array_position % max_number_of_items;
        current_action_item = stack_items[real_array_position];

        free_action_item(current_action_item);

        ++current_number_of_items;
        --current_array_position;
      }
    } break;
    default:
    {

    } break;
  }

  *stack_begin = 0;
  *stack_num_items = 0;
}

void clear_events(text_id tid)
{
  empty_stack(tid, HO_UNDO_STACK);
  empty_stack(tid, HO_REDO_STACK);
}
