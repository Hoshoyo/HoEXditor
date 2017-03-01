#include "text_events.h"
#include "input.h"
#include "text_manager.h"
#include "memory.h"
#include "editor.h"
#include "util.h"
#include "os_dependent.h"

extern Editor_State editor_state;
ho_text_events main_text_events;

s32 init_text_events()
{
  main_text_events.num_undo_items = 0;
  main_text_events.num_redo_items = 0;
  main_text_events.undo_stack_begin = 0;
  main_text_events.redo_stack_begin = 0;
  main_text_events.max_undo_items = MAX_UNDO_MEM;
  main_text_events.max_redo_items = MAX_REDO_MEM;
  main_text_events.num_action_commands = 0;

  return 0;
}

s32 save_file(u8* filename)
{
  u64 size;
	u8* text = get_text_as_contiguous_memory(&size);

	s32 written_bytes = write_file(filename, text, size);
  hfree(text);

  if (written_bytes == size)
    return 0;
  else
    return -1;
}

void keyboard_call_events()
{
  u32 i, j;

  for (i=0; i<main_text_events.num_action_commands; ++i)
  {
    // testar se todas keys foram pressionadas!
    for (j=0; j<main_text_events.action_commands[i].num_associated_keys; ++j)
      if (!keyboard_state.key[main_text_events.action_commands[i].associated_keys[j]])
        break;

    if (j == main_text_events.action_commands[i].num_associated_keys)
      execute_action_command(main_text_events.action_commands[i].type);
  }
}

void execute_action_command(enum ho_action_command_type type)
{
  switch (type)
  {
    case HO_UNDO:
    {
      do_undo();
    } break;
    case HO_REDO:
    {
      do_redo();
    } break;
    case HO_COPY:
    {
      open_clipboard();
      set_clipboard_content("Voce esta tentando copiar um texto do HoEXditor. Infelizmente isto ainda nao eh possivel, pois o programador responsavel ainda nao implementou a selecao de texto.",
    sizeof("Voce esta tentando copiar um texto do HoEXditor. Infelizmente isto ainda nao eh possivel, pois o programador responsavel ainda nao implementou a selecao de texto."));
      close_clipboard();
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

      insert_text(new_text, text_size, editor_state.cursor_info.cursor_offset);
      add_undo_item(HO_INSERT_TEXT, new_text, text_size, editor_state.cursor_info.cursor_offset);
      editor_state.cursor_info.cursor_offset += text_size;

      close_clipboard();
    } break;
  }
}

void add_undo_item(enum ho_action_type type, u8* text, u64 text_size, u64 cursor_position)
{
  ho_aiv_undo_redo* aiv = halloc(sizeof(ho_aiv_undo_redo));
  aiv->text = text;
  aiv->text_size = text_size;
  aiv->cursor_position = cursor_position;

  ho_action_item action_item;
  action_item.type = type;
  action_item.value = aiv;
  push_stack_item(HO_UNDO_STACK, action_item);

  // empty redo stack.
  empty_stack(HO_REDO_STACK);
}

void add_redo_item(enum ho_action_type type, u8* text, u64 text_size, u64 cursor_position)
{
  ho_aiv_undo_redo* aiv = halloc(sizeof(ho_aiv_undo_redo));
  aiv->text = text;
  aiv->text_size = text_size;
  aiv->cursor_position = editor_state.cursor_info.cursor_offset;

  ho_action_item action_item;
  action_item.type = type;
  action_item.value = aiv;
  push_stack_item(HO_REDO_STACK, action_item);
}

void update_action_command(enum ho_action_command_type type, u32 num_associated_keys, u32* associated_keys)
{
  for (u32 i=0; i<main_text_events.num_action_commands; ++i)
    if (type == main_text_events.action_commands[i].type)
    {
      main_text_events.action_commands[i].num_associated_keys = num_associated_keys;
      for (u32 j=0; j<num_associated_keys; ++j)
        main_text_events.action_commands[i].associated_keys[j] = associated_keys[j];
      return;
    }

  if (main_text_events.num_action_commands == MAX_ACTION_COMMANDS)
  {
    error_warning("Error: Action Command could not be added. No space.\n");
    return;
  }

  ho_action_command action_command;

  action_command.type = type;
  action_command.num_associated_keys = num_associated_keys;

  for (u32 j=0; j<num_associated_keys; ++j)
    action_command.associated_keys[j] = associated_keys[j];

  main_text_events.action_commands[main_text_events.num_action_commands] = action_command;
  ++main_text_events.num_action_commands;
}

void remove_action_command(enum ho_action_command_type type)
{

}

void fill_stack_attrbs(HO_EVENT_STACK stack, u32** stack_begin, u32** stack_max_items, u32** stack_num_items, ho_action_item** stack_items)
{
  switch (stack)
  {
    case HO_UNDO_STACK:
      *stack_begin = &(main_text_events.undo_stack_begin);
      *stack_max_items = &(main_text_events.max_undo_items);
      *stack_num_items = &(main_text_events.num_undo_items);
      *stack_items = main_text_events.undo_items;
      break;
    case HO_REDO_STACK:
      *stack_begin = &(main_text_events.redo_stack_begin);
      *stack_max_items = &(main_text_events.max_redo_items);
      *stack_num_items = &(main_text_events.num_redo_items);
      *stack_items = main_text_events.redo_items;
      break;
  }
}

s32 push_stack_item(HO_EVENT_STACK stack, ho_action_item item)
{
  u32 *stack_begin, *stack_max_items, *stack_num_items;
  ho_action_item* stack_items;
  ho_action_item old_item;

  fill_stack_attrbs(stack, &stack_begin, &stack_max_items, &stack_num_items, &stack_items);

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

ho_action_item pop_stack_item(HO_EVENT_STACK stack)
{
  u32 *stack_begin, *stack_max_items, *stack_num_items;
  ho_action_item* stack_items;
  ho_action_item item;

  fill_stack_attrbs(stack, &stack_begin, &stack_max_items, &stack_num_items, &stack_items);

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

void print_stack(HO_EVENT_STACK stack)
{
  u32 *stack_begin, *stack_max_items, *stack_num_items;
  ho_action_item* stack_items;

  fill_stack_attrbs(stack, &stack_begin, &stack_max_items, &stack_num_items, &stack_items);

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

void do_undo()
{
  if (is_stack_empty(HO_UNDO_STACK))
    return;

  ho_action_item action_item = pop_stack_item(HO_UNDO_STACK);
  push_stack_item(HO_REDO_STACK, copy_action_item(action_item));

  switch (action_item.type)
  {
    case HO_INSERT_TEXT:
    {
      ho_aiv_undo_redo* aiv = (ho_aiv_undo_redo*) action_item.value;
      u32 cursor_position = aiv->cursor_position;
      u32 text_size = aiv->text_size;
      u8* text = aiv->text;
      delete_text(null, text_size, cursor_position);
      editor_state.cursor_info.cursor_offset -= text_size;
      free_action_item(action_item);
    } break;
    case HO_DELETE_TEXT:
    {
      ho_aiv_undo_redo* aiv = (ho_aiv_undo_redo*) action_item.value;
      u32 cursor_position = aiv->cursor_position;
      u32 text_size = aiv->text_size;
      u8* text = aiv->text;
      insert_text(text, text_size, cursor_position);
      editor_state.cursor_info.cursor_offset += text_size;
      free_action_item(action_item);
    } break;
  }
}

void do_redo()
{
  if (is_stack_empty(HO_REDO_STACK))
    return;

  ho_action_item action_item = pop_stack_item(HO_REDO_STACK);
  push_stack_item(HO_UNDO_STACK, copy_action_item(action_item));

  switch (action_item.type)
  {
    case HO_INSERT_TEXT:
    {
      ho_aiv_undo_redo* aiv = (ho_aiv_undo_redo*) action_item.value;
      u32 cursor_position = aiv->cursor_position;
      u32 text_size = aiv->text_size;
      u8* text = aiv->text;
      insert_text(text, text_size, cursor_position);
      editor_state.cursor_info.cursor_offset += text_size;
      free_action_item(action_item);
    } break;
    case HO_DELETE_TEXT:
    {
      ho_aiv_undo_redo* aiv = (ho_aiv_undo_redo*) action_item.value;
      u32 cursor_position = aiv->cursor_position;
      u32 text_size = aiv->text_size;
      u8* text = aiv->text;
      delete_text(null, text_size, cursor_position);
      editor_state.cursor_info.cursor_offset -= text_size;
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

bool is_stack_empty(HO_EVENT_STACK stack)
{
  u32 *stack_begin, *stack_max_items, *stack_num_items;
  ho_action_item* stack_items;

  fill_stack_attrbs(stack, &stack_begin, &stack_max_items, &stack_num_items, &stack_items);

  if (*stack_num_items == 0)
    return true;

  return false;
}

void empty_stack(HO_EVENT_STACK stack)
{
  u32 *stack_begin, *stack_max_items, *stack_num_items;
  ho_action_item* stack_items;

  fill_stack_attrbs(stack, &stack_begin, &stack_max_items, &stack_num_items, &stack_items);

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

void clear_events()
{
  empty_stack(HO_UNDO_STACK);
  empty_stack(HO_REDO_STACK);
}
