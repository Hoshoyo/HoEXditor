#include "interface_events.h"
#include "interface.h"

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
      MessageBox(0, "'New' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_SUBMENU_ITEM_1_2: {
		ui_show_open_file_dialog();
    } break;
    case T_UI_SUBMENU_ITEM_1_3: {
      MessageBox(0, "'Save' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_SUBMENU_ITEM_1_4: {
      MessageBox(0, "'Save as...' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_SUBMENU_ITEM_1_5: {
      MessageBox(0, "'Settings' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_SUBMENU_ITEM_1_6: {
      MessageBox(0, "'Close' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_SUBMENU_ITEM_1_7: {
      MessageBox(0, "'Close All' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_SUBMENU_ITEM_1_8: {
      MessageBox(0, "'Exit' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_SUBMENU_ITEM_2_1: {
      MessageBox(0, "'Undo' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_SUBMENU_ITEM_2_2: {
      MessageBox(0, "'Redo' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_SUBMENU_ITEM_2_3: {
      MessageBox(0, "'Cut' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_SUBMENU_ITEM_2_4: {
      MessageBox(0, "'Copy' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_SUBMENU_ITEM_2_5: {
      MessageBox(0, "'Paste' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_SUBMENU_ITEM_2_6: {
      MessageBox(0, "'Select All' handle called", "Hoshoyo's MessageBox Information", MB_OK);
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
      MessageBox(0, "'HEX Mode' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_SUBMENU_ITEM_3_2: {
      MessageBox(0, "'ASCII Mode' handle called", "Hoshoyo's MessageBox Information", MB_OK);
    } break;
    case T_UI_SUBMENU_ITEM_3_3: {
      MessageBox(0, "'Binary Mode' handle called", "Hoshoyo's MessageBox Information", MB_OK);
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
