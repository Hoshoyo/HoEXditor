#include "text_events.h"
#include "input.h"
#include "text_manager.h"
#include "memory.h"
#include "editor.h"
#include "util.h"
#include "os_dependent.h"

#define MOD(n) (n) > 0 ? (n) : -(n)

extern Editor_State* editor_state;
ho_text_events* _te_text_events[MAX_FILES_OPEN];

s32 init_text_events(s32 id)
{
  _te_text_events[id] = halloc(sizeof(ho_text_events));
  _te_text_events[id]->num_undo_items = 0;
  _te_text_events[id]->num_redo_items = 0;
  _te_text_events[id]->undo_stack_begin = 0;
  _te_text_events[id]->redo_stack_begin = 0;
  _te_text_events[id]->max_undo_items = MAX_UNDO_MEM;
  _te_text_events[id]->max_redo_items = MAX_REDO_MEM;
  _te_text_events[id]->num_action_commands = 0;

  return 0;
}

s32 finalize_text_events(s32 id)
{
  clear_events(id);
  hfree(_te_text_events[id]);

  return 0;
}

s32 save_file(s32 id, u8* filename)
{
  u64 size;
	u8* text = get_text_as_contiguous_memory(id, &size);

	s32 written_bytes = write_file(filename, text, size);
  hfree(text);

  if (written_bytes == size)
    return 0;
  else
    return -1;
}

void keyboard_call_events(s32 id)
{
  u32 i, j;

  for (i=0; i<_te_text_events[id]->num_action_commands; ++i)
  {
    // testar se todas keys foram pressionadas!
    for (j=0; j<_te_text_events[id]->action_commands[i].num_associated_keys; ++j)
      if (!keyboard_state.key[_te_text_events[id]->action_commands[i].associated_keys[j]])
        break;

    if (j == _te_text_events[id]->action_commands[i].num_associated_keys)
      execute_action_command(id, _te_text_events[id]->action_commands[i].type);
  }
}

void execute_action_command(s32 id, enum ho_action_command_type type)
{
  switch (type)
  {
    case HO_UNDO:
    {
      do_undo(id);
    } break;
    case HO_REDO:
    {
      do_redo(id);
    } break;
    case HO_COPY:
    {
      if (editor_state->selecting)
      {
        s64 bytes_to_copy = MOD(editor_state->cursor_info.selection_offset - editor_state->cursor_info.cursor_offset);
        s64 cursor_begin = MIN(editor_state->cursor_info.selection_offset, editor_state->cursor_info.cursor_offset);

        if (bytes_to_copy > 0)
        {
          open_clipboard();
          u32 block_position;
          u8* text_to_copy = halloc(bytes_to_copy * sizeof(u8));
          ho_block* block = get_initial_block_at_cursor(id, &block_position, cursor_begin);
          move_block_data(block, block_position, bytes_to_copy, text_to_copy);
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

      insert_text(id, new_text, text_size, editor_state->cursor_info.cursor_offset);
      add_undo_item(id, HO_INSERT_TEXT, new_text, text_size, editor_state->cursor_info.cursor_offset);
      editor_state->cursor_info.cursor_offset += text_size;

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
  }
}

void handle_char_press(s32 id, u8 key)
{
  // in there's a selection, delete it
  if (editor_state->selecting)
  {
    s64 bytes_to_delete = MOD(editor_state->cursor_info.selection_offset - editor_state->cursor_info.cursor_offset);
    s64 cursor_begin = MIN(editor_state->cursor_info.selection_offset, editor_state->cursor_info.cursor_offset);
    s64 move_cursor = (editor_state->cursor_info.selection_offset > editor_state->cursor_info.cursor_offset) ? 0 : bytes_to_delete;

    u8* deleted_text = halloc(bytes_to_delete * sizeof(u8));

    delete_text(id, deleted_text, bytes_to_delete * sizeof(u8), cursor_begin);
    add_undo_item(id, HO_DELETE_TEXT, deleted_text, bytes_to_delete * sizeof(u8), cursor_begin);

    editor_state->cursor_info.cursor_offset -= move_cursor;
  }

  switch (key)
  {
    case CARRIAGE_RETURN_KEY:
    {
      // temporary - this is OS dependent
      u8* inserted_text = halloc(sizeof(u8));
      inserted_text[0] = LINE_FEED_KEY;

      insert_text(id, inserted_text, 1, editor_state->cursor_info.cursor_offset);
      add_undo_item(id, HO_INSERT_TEXT, inserted_text, 1 * sizeof(u8), editor_state->cursor_info.cursor_offset);

      editor_state->cursor_info.cursor_offset += 1;
    } break;
    case BACKSPACE_KEY:
    {
      if (!editor_state->selecting && editor_state->cursor_info.cursor_offset > 0)
      {
      	u8* deleted_text = halloc(sizeof(u8));

      	delete_text(id, deleted_text, sizeof(u8), editor_state->cursor_info.cursor_offset - 1);
      	add_undo_item(id, HO_DELETE_TEXT, deleted_text, sizeof(u8), editor_state->cursor_info.cursor_offset - 1);

      	editor_state->cursor_info.cursor_offset -= 1;
      }
    } break;
    default:
    {
      u8* inserted_text = halloc(sizeof(u8));
      *inserted_text = key;

      insert_text(id, inserted_text, 1, editor_state->cursor_info.cursor_offset);
      add_undo_item(id, HO_INSERT_TEXT, inserted_text, sizeof(u8), editor_state->cursor_info.cursor_offset);

      editor_state->cursor_info.cursor_offset += 1;
    } break;
  }

	check_text(id);
	check_arenas(id);
}

bool test_if_pattern_match(ho_block* block, u32 block_position, u8* pattern, u64 pattern_length)
{
  ho_block_container* current_block_container = block->container;
  u32 current_block_position = block->position_in_container;
  ho_block* current_block = block;
  u32 pattern_position = 0;

  while (current_block_container != null)
  {
    current_block = &current_block_container->blocks[current_block_position];
    for (; current_block_position < current_block_container->num_blocks_in_container; ++current_block_position)
    {
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

ho_search_result* search_word(s32 id, u64 cursor_begin, u64 cursor_end, u8* pattern, u64 pattern_length)
{
  ho_search_result* result = null;
  ho_search_result* last_result = null;
  u32 block_position;
  ho_block* current_block = get_initial_block_at_cursor(id, &block_position, cursor_begin);
  ho_block_container* current_block_container = current_block->container;
  u64 current_cursor_position = cursor_begin;
  u32 current_block_position = current_block->position_in_container;

  if (pattern_length == 0 || pattern_length > _tm_text_size[id])
    return null;

  while (current_cursor_position <= (cursor_end - pattern_length))
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

        ++current_cursor_position;
      }
      block_position = 0;
    }
    current_block_position = 0;
    current_block_container = current_block_container->next;
  }

  return result;
}

void add_undo_item(s32 id, enum ho_action_type type, u8* text, u64 text_size, u64 cursor_position)
{
  ho_aiv_undo_redo* aiv = halloc(sizeof(ho_aiv_undo_redo));
  aiv->text = text;
  aiv->text_size = text_size;
  aiv->cursor_position = cursor_position;

  ho_action_item action_item;
  action_item.type = type;
  action_item.value = aiv;
  push_stack_item(id, HO_UNDO_STACK, action_item);

  // empty redo stack.
  empty_stack(id, HO_REDO_STACK);
}

void add_redo_item(s32 id, enum ho_action_type type, u8* text, u64 text_size, u64 cursor_position)
{
  ho_aiv_undo_redo* aiv = halloc(sizeof(ho_aiv_undo_redo));
  aiv->text = text;
  aiv->text_size = text_size;
  aiv->cursor_position = editor_state->cursor_info.cursor_offset;

  ho_action_item action_item;
  action_item.type = type;
  action_item.value = aiv;
  push_stack_item(id, HO_REDO_STACK, action_item);
}

void update_action_command(s32 id, enum ho_action_command_type type, u32 num_associated_keys, u32* associated_keys)
{
  for (u32 i=0; i<_te_text_events[id]->num_action_commands; ++i)
    if (type == _te_text_events[id]->action_commands[i].type)
    {
      _te_text_events[id]->action_commands[i].num_associated_keys = num_associated_keys;
      for (u32 j=0; j<num_associated_keys; ++j)
        _te_text_events[id]->action_commands[i].associated_keys[j] = associated_keys[j];
      return;
    }

  if (_te_text_events[id]->num_action_commands == MAX_ACTION_COMMANDS)
  {
    error_warning("Error: Action Command could not be added. No space.\n");
    return;
  }

  ho_action_command action_command;

  action_command.type = type;
  action_command.num_associated_keys = num_associated_keys;

  for (u32 j=0; j<num_associated_keys; ++j)
    action_command.associated_keys[j] = associated_keys[j];

  _te_text_events[id]->action_commands[_te_text_events[id]->num_action_commands] = action_command;
  ++_te_text_events[id]->num_action_commands;
}

void remove_action_command(s32 id, enum ho_action_command_type type)
{

}

void fill_stack_attrbs(s32 id, HO_EVENT_STACK stack, u32** stack_begin, u32** stack_max_items, u32** stack_num_items, ho_action_item** stack_items)
{
  switch (stack)
  {
    case HO_UNDO_STACK:
      *stack_begin = &(_te_text_events[id]->undo_stack_begin);
      *stack_max_items = &(_te_text_events[id]->max_undo_items);
      *stack_num_items = &(_te_text_events[id]->num_undo_items);
      *stack_items = _te_text_events[id]->undo_items;
      break;
    case HO_REDO_STACK:
      *stack_begin = &(_te_text_events[id]->redo_stack_begin);
      *stack_max_items = &(_te_text_events[id]->max_redo_items);
      *stack_num_items = &(_te_text_events[id]->num_redo_items);
      *stack_items = _te_text_events[id]->redo_items;
      break;
  }
}

s32 push_stack_item(s32 id, HO_EVENT_STACK stack, ho_action_item item)
{
  u32 *stack_begin, *stack_max_items, *stack_num_items;
  ho_action_item* stack_items;
  ho_action_item old_item;

  fill_stack_attrbs(id, stack, &stack_begin, &stack_max_items, &stack_num_items, &stack_items);

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

ho_action_item pop_stack_item(s32 id, HO_EVENT_STACK stack)
{
  u32 *stack_begin, *stack_max_items, *stack_num_items;
  ho_action_item* stack_items;
  ho_action_item item;

  fill_stack_attrbs(id, stack, &stack_begin, &stack_max_items, &stack_num_items, &stack_items);

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

void print_stack(s32 id, HO_EVENT_STACK stack)
{
  u32 *stack_begin, *stack_max_items, *stack_num_items;
  ho_action_item* stack_items;

  fill_stack_attrbs(id, stack, &stack_begin, &stack_max_items, &stack_num_items, &stack_items);

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

void do_undo(s32 id)
{
  if (is_stack_empty(id, HO_UNDO_STACK))
    return;

  ho_action_item action_item = pop_stack_item(id, HO_UNDO_STACK);
  push_stack_item(id, HO_REDO_STACK, copy_action_item(action_item));

  switch (action_item.type)
  {
    case HO_INSERT_TEXT:
    {
      ho_aiv_undo_redo* aiv = (ho_aiv_undo_redo*) action_item.value;
      u32 cursor_position = aiv->cursor_position;
      u32 text_size = aiv->text_size;
      u8* text = aiv->text;
      delete_text(id, null, text_size, cursor_position);
      editor_state->cursor_info.cursor_offset -= text_size;
      free_action_item(action_item);
    } break;
    case HO_DELETE_TEXT:
    {
      ho_aiv_undo_redo* aiv = (ho_aiv_undo_redo*) action_item.value;
      u32 cursor_position = aiv->cursor_position;
      u32 text_size = aiv->text_size;
      u8* text = aiv->text;
      insert_text(id, text, text_size, cursor_position);
      editor_state->cursor_info.cursor_offset += text_size;
      free_action_item(action_item);
    } break;
  }
}

void do_redo(s32 id)
{
  if (is_stack_empty(id, HO_REDO_STACK))
    return;

  ho_action_item action_item = pop_stack_item(id, HO_REDO_STACK);
  push_stack_item(id, HO_UNDO_STACK, copy_action_item(action_item));

  switch (action_item.type)
  {
    case HO_INSERT_TEXT:
    {
      ho_aiv_undo_redo* aiv = (ho_aiv_undo_redo*) action_item.value;
      u32 cursor_position = aiv->cursor_position;
      u32 text_size = aiv->text_size;
      u8* text = aiv->text;
      insert_text(id, text, text_size, cursor_position);
      editor_state->cursor_info.cursor_offset += text_size;
      free_action_item(action_item);
    } break;
    case HO_DELETE_TEXT:
    {
      ho_aiv_undo_redo* aiv = (ho_aiv_undo_redo*) action_item.value;
      u32 cursor_position = aiv->cursor_position;
      u32 text_size = aiv->text_size;
      u8* text = aiv->text;
      delete_text(id, null, text_size, cursor_position);
      editor_state->cursor_info.cursor_offset -= text_size;
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

bool is_stack_empty(s32 id, HO_EVENT_STACK stack)
{
  u32 *stack_begin, *stack_max_items, *stack_num_items;
  ho_action_item* stack_items;

  fill_stack_attrbs(id, stack, &stack_begin, &stack_max_items, &stack_num_items, &stack_items);

  if (*stack_num_items == 0)
    return true;

  return false;
}

void empty_stack(s32 id, HO_EVENT_STACK stack)
{
  u32 *stack_begin, *stack_max_items, *stack_num_items;
  ho_action_item* stack_items;

  fill_stack_attrbs(id, stack, &stack_begin, &stack_max_items, &stack_num_items, &stack_items);

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

void clear_events(s32 id)
{
  empty_stack(id, HO_UNDO_STACK);
  empty_stack(id, HO_REDO_STACK);
}
