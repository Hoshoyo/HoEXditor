#include "input.h"
#include "editor.h"
#include "interface.h"

void handle_key_down(s32 key, s32 mod)
{
  Editor_State* es = ui_get_focused_editor();
  keyboard_state.key[key] = true;

  if (es != null)
  {
	  if (key == VK_UP || key == VK_DOWN || key == VK_RIGHT || key == VK_LEFT || key == VK_HOME || key == VK_END)
	  {
		  if (keyboard_state.key[VK_SHIFT])
			  editor_start_selection(es);
		  else if (!keyboard_state.key[BACKSPACE_KEY] && !keyboard_state.key[CTRL_KEY])
			  editor_reset_selection(es);
	  }

	  switch (key)
	  {
		  case VK_UP: {
			  cursor_up(es, 1);
		  } break;
		  case VK_DOWN: {
			  cursor_down(es, 1);
		  } break;
		  case VK_LEFT: {
			  cursor_left(es, 1);
		  } break;
		  case VK_RIGHT: {
			  cursor_right(es, 1);
		  } break;
		  case VK_HOME: {
			  cursor_home(es, 1);
		  } break;
		  case VK_END: {
			  cursor_end(es, 1);
		  } break;
	  }

	  keyboard_call_events(es);
  }

  ui_handle_key_down(key);

  if (key == VK_SHIFT && es != null)
    editor_start_selection(es);
}

void handle_char_down(s32 key)
{
  Editor_State* es = ui_get_focused_editor();

  if (!keyboard_state.key[CTRL_KEY] && es != null) {
    if (es->individual_char_handler == null || es->individual_char_handler(es, key))
    {
      handle_char_press(es, key);
      editor_reset_selection(es);
    }
  }
}

void handle_key_up(s32 key)
{
  Editor_State* es = ui_get_focused_editor();

  keyboard_state.key[key] = false;
  if (key == VK_SHIFT && es != null)
    editor_end_selection(es);
}

void handle_mouse_move(s32 x, s32 y)
{

}

void handle_lmouse_down(s32 x, s32 y)
{
  Editor_State* es = ui_get_focused_editor();

  if (es != null)
  {
    if (!keyboard_state.key[VK_SHIFT])
      editor_reset_selection(es);
  }

  if (is_interface_initialized)
    ui_handle_mouse_click(x, y);

  print("x: %d, y: %d\n", x, y);
}

void handle_rmouse_down(s32 x, s32 y)
{

}

void handle_file_drop(s32 x, s32 y, u8* path)
{
  print("Attempted to drop file (%s) at mouse location {%d, %d}.\n", path, x, y);
  ui_handle_file_drop(path, x, y);
}
