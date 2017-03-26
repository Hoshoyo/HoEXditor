#include "interface_events.h"
#include "interface.h"
#include "text_events.h"

void handle_top_menu_event(enum interface_sub_menu_item_type code)
{
  switch (code)
  {
    case T_UI_MENU_ITEM_1: {
      MessageBox(0, "'File' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_MENU_ITEM_2: {
      MessageBox(0, "'Edit' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_MENU_ITEM_3: {
      MessageBox(0, "'View' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_MENU_ITEM_4: {
      MessageBox(0, "'Help' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_SUBMENU_ITEM_1_1: {
		ui_open_file(true, null);
    } break;
    case T_UI_SUBMENU_ITEM_1_2: {
		ui_show_open_file_dialog();
    } break;
    case T_UI_SUBMENU_ITEM_1_3: {
		Editor_State* es = ui_get_focused_editor();
		if (es != null)
			execute_action_command(es, HO_SAVE);
    } break;
    case T_UI_SUBMENU_ITEM_1_4: {
		ui_show_save_file_dialog();
    } break;
    case T_UI_SUBMENU_ITEM_1_5: {
		MessageBox(0, "Bad memory access in kernel's module.\n\nIn case proccess is still active, BIOS CMOS RAM memory may be overwritten.\n\nTarget: kernel32.dll\nError Code: 0xFD7FFFFFF", "WINDOWS CRITICAL ERROR", MB_ICONERROR);
    } break;
    case T_UI_SUBMENU_ITEM_1_6: {
		ui_close_file();
    } break;
    case T_UI_SUBMENU_ITEM_1_7: {
		ui_close_all_files();
    } break;
    case T_UI_SUBMENU_ITEM_1_8: {
		// What an elegant solution
		ExitProcess(0);
    } break;
    case T_UI_SUBMENU_ITEM_2_1: {
		Editor_State* es = ui_get_focused_editor();
		if (es != null)
			execute_action_command(es, HO_UNDO);
    } break;
    case T_UI_SUBMENU_ITEM_2_2: {
		Editor_State* es = ui_get_focused_editor();
		if (es != null)
			execute_action_command(es, HO_REDO);
    } break;
    case T_UI_SUBMENU_ITEM_2_3: {
		Editor_State* es = ui_get_focused_editor();
		if (es != null)
			execute_action_command(es, HO_CUT);
    } break;
    case T_UI_SUBMENU_ITEM_2_4: {
		Editor_State* es = ui_get_focused_editor();
		if (es != null)
			execute_action_command(es, HO_COPY);
    } break;
    case T_UI_SUBMENU_ITEM_2_5: {
		Editor_State* es = ui_get_focused_editor();
		if (es != null)
			execute_action_command(es, HO_PASTE);
    } break;
    case T_UI_SUBMENU_ITEM_2_6: {
		Editor_State* es = ui_get_focused_editor();
		if (es != null)
			editor_select_all(es);
    } break;
    case T_UI_SUBMENU_ITEM_2_7: {
      MessageBox(0, "'Find...' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_SUBMENU_ITEM_2_8: {
      MessageBox(0, "'Find and Replace...' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_SUBMENU_ITEM_2_9: {
      MessageBox(0, "'Go To Line...' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_SUBMENU_ITEM_3_1: {
		Editor_State* es = ui_get_focused_editor();
		if (es != null)
			es->mode = EDITOR_MODE_HEX;
    } break;
    case T_UI_SUBMENU_ITEM_3_2: {
		Editor_State* es = ui_get_focused_editor();
		if (es != null)
			es->mode = EDITOR_MODE_ASCII;
    } break;
    case T_UI_SUBMENU_ITEM_3_3: {
		Editor_State* es = ui_get_focused_editor();
		if (es != null)
			es->mode = EDITOR_MODE_BINARY;
    } break;
    case T_UI_SUBMENU_ITEM_3_4: {
      MessageBox(0, "'Increase Font Size' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_SUBMENU_ITEM_3_5: {
      MessageBox(0, "'Decrease Font Size' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_SUBMENU_ITEM_4_1: {
      MessageBox(0, "'About' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    default: {
      MessageBox(0, "'Default' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
  }
}
