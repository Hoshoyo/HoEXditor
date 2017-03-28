#ifndef HOHEX_INTERFACE_DEFINITIONS_H
#define HOHEX_INTERFACE_DEFINITIONS_H

#define UI_ICON_PATH "./res/icon.png"

#define UI_TITLE "HoEXditor - Your hexadecimal editor."
#define UI_MENU_ITEM_1 "File"
#define UI_MENU_ITEM_2 "Edit"
#define UI_MENU_ITEM_3 "View"
#define UI_MENU_ITEM_4 "Help"

#define UI_SUBMENU_ITEM_1_1 "New"
#define UI_SUBMENU_ITEM_1_2 "Open..."
#define UI_SUBMENU_ITEM_1_3 "Save"
#define UI_SUBMENU_ITEM_1_4 "Save as..."
#define UI_SUBMENU_ITEM_1_5 "Settings"
#define UI_SUBMENU_ITEM_1_6 "Close"
#define UI_SUBMENU_ITEM_1_7 "Close All"
#define UI_SUBMENU_ITEM_1_8 "Exit"

#define UI_SUBMENU_ITEM_2_1 "Undo"
#define UI_SUBMENU_ITEM_2_2 "Redo"
#define UI_SUBMENU_ITEM_2_3 "Cut"
#define UI_SUBMENU_ITEM_2_4 "Copy"
#define UI_SUBMENU_ITEM_2_5 "Paste"
#define UI_SUBMENU_ITEM_2_6 "Select All"
#define UI_SUBMENU_ITEM_2_7 "Find..."
#define UI_SUBMENU_ITEM_2_8 "Find and Replace..."
#define UI_SUBMENU_ITEM_2_9 "Go To Line..."

#define UI_SUBMENU_ITEM_3_1 "HEX Mode"
#define UI_SUBMENU_ITEM_3_2 "ASCII Mode"
#define UI_SUBMENU_ITEM_3_3 "Binary Mode"
#define UI_SUBMENU_ITEM_3_4 "Increase Font Size"
#define UI_SUBMENU_ITEM_3_5 "Decrease Font Size"
#define UI_SUBMENU_ITEM_3_6 "Show Console"
#define UI_SUBMENU_ITEM_3_7 "Split View"

#define UI_SUBMENU_ITEM_4_1 "About"

#define UI_DEFAULT_FILE_NAME "Unnamed"

#define UI_TOP_HEADER_HEIGHT 0//35.0f
#define UI_TOP_MENU_HEIGHT 25.0f
#define UI_FILE_SWITCH_AREA_HEIGHT 35.0f
#define UI_LEFT_COLUMN_WIDTH 2.0f
#define UI_RIGHT_COLUMN_WIDTH 2.0f
#define UI_FOOTER_HEIGHT 2.0f
#define UI_TEXT_PADDING 5.0f

#define UI_CONSOLE_VIEW_HEIGHT 190.0f
#define UI_CONSOLE_INPUT_HEIGHT 25.0f
#define UI_CONSOLE_VIEW_BUFFER_SIZE 1024
#define UI_CONSOLE_INPUT_BUFFER_SIZE 1024

#define UI_SEARCH_INPUT_HEIGHT 25.0f
#define UI_SEARCH_INPUT_BUFFER_SIZE 1024
#define UI_SEARCH_VIEW_HEIGHT 25.0f
#define UI_SEARCH_VIEW_BUFFER_SIZE 1024

#if HACKER_THEME
#define UI_BACKGROUND_COLOR (vec4) {45/255.0f, 45/255.0f, 48/255.0f, 255/255.0f}
#define UI_TEXT_AREA_COLOR (vec4) {0/255.0f, 0/255.0f, 0/255.0f, 255/255.0f}
#define UI_TOP_MENU_TEXT_COLOR (vec4) {0/255.0f, 255/255.0f, 0/255.0f, 255/255.0f}
#define UI_FILE_SWITCH_AREA_ITEM_BACKGROUND (vec4) {0/255.0f, 155.0f/255.0f, 0/255.0f, 255/255.0f}
#define UI_FILE_SWITCH_AREA_SELECTED_ITEM_BACKGROUND (vec4) {0/255.0f, 200.0f/255.0f, 0/255.0f, 255/255.0f}
#define UI_TOP_MENU_SELECTION_COLOR (vec4) {20/255.0f, 20/255.0f, 20/255.0f, 255/255.0f}
#define UI_SUB_MENU_SELECTION_COLOR (vec4) {0/255.0f, 0/255.0f, 255/255.0f, 255/255.0f}
#define UI_FILE_SWITCH_AREA_TEXT_COLOR (vec4) {255/255.0f, 255/255.0f, 255/255.0f, 255/255.0f}
#define UI_MAIN_TEXT_COLOR (vec4) { 0.0f, 0.9f, 0.0f, 1.0f }
#define UI_MAIN_TEXT_CURSOR_COLOR (vec4) { 1.0f, 1.0f, 1.0f, 0.5f }
#define UI_MAIN_TEXT_LINE_NUMBER_COLOR (vec4) { 1.0f, 1.0f, 1.0f, 0.5f }
#define UI_CONSOLE_VIEW_TEXT_COLOR (vec4) { 0.0f, 0.9f, 0.0f, 1.0f }
#define UI_CONSOLE_VIEW_CURSOR_COLOR (vec4) { 1.0f, 1.0f, 1.0f, 0.5f }
#define UI_CONSOLE_VIEW_BACKGROUND_COLOR (vec4) {45/255.0f, 45/255.0f, 48/255.0f, 255/255.0f}
#define UI_CONSOLE_INPUT_TEXT_COLOR (vec4) { 0.0f, 0.9f, 0.0f, 1.0f }
#define UI_CONSOLE_INPUT_CURSOR_COLOR (vec4) { 1.0f, 1.0f, 1.0f, 0.5f }
#define UI_CONSOLE_INPUT_BACKGROUND_COLOR (vec4) {45/255.0f, 45/255.0f, 48/255.0f, 255/255.0f}
#elif WHITE_THEME
#define UI_BACKGROUND_COLOR (vec4) {180.0f/255.0f, 180.0f/255.0f, 180.0f/255.0f, 255/255.0f}
#define UI_TEXT_AREA_COLOR (vec4) {230.0f/255.0f, 230.0f/255.0f, 230.0f/255.0f, 255/255.0f}
#define UI_TOP_MENU_TEXT_COLOR (vec4) {10.0f/255.0f, 10.0f/255.0f, 10.0f/255.0f, 255/255.0f}
#define UI_FILE_SWITCH_AREA_ITEM_BACKGROUND (vec4) {170.0f/255.0f, 170.0f/255.0f, 170.0f/255.0f, 255.0f/255.0f}
#define UI_FILE_SWITCH_AREA_SELECTED_ITEM_BACKGROUND (vec4) {230.0f/255.0f, 230.0f/255.0f, 230.0f/255.0f, 255.0f/255.0f}
#define UI_TOP_MENU_SELECTION_COLOR (vec4) {230.0f/255.0f, 230.0f/255.0f, 230.0f/255.0f, 255/255.0f}
#define UI_SUB_MENU_SELECTION_COLOR (vec4) {200.0f/255.0f, 200.0f/255.0f, 200.0f/255.0f, 255/255.0f}
#define UI_FILE_SWITCH_AREA_TEXT_COLOR (vec4) {50.0f/255.0f, 50.0f/255.0f, 50.0f/255.0f, 255/255.0f}
#define UI_MAIN_TEXT_COLOR (vec4) { 0.1f, 0.1f, 0.1f, 1.0f }
#define UI_MAIN_TEXT_CURSOR_COLOR (vec4) { 0.0f, 0.0f, 0.0f, 0.8f }
#define UI_MAIN_TEXT_LINE_NUMBER_COLOR (vec4) { 0.0f, 0.0f, 0.0f, 0.8f }
#define UI_CONSOLE_VIEW_TEXT_COLOR (vec4) { 0.1f, 0.1f, 0.1f, 1.0f }
#define UI_CONSOLE_VIEW_CURSOR_COLOR (vec4) { 0.1f, 0.1f, 0.1f, 0.5f }
#define UI_CONSOLE_VIEW_BACKGROUND_COLOR (vec4) { 0.7f, 0.7f, 0.7f, 1.0f}
#define UI_CONSOLE_INPUT_TEXT_COLOR (vec4) { 0.1f, 0.1f, 0.1f, 1.0f }
#define UI_CONSOLE_INPUT_CURSOR_COLOR (vec4) { 0.1f, 0.1f, 0.1f, 0.5f }
#define UI_CONSOLE_INPUT_BACKGROUND_COLOR (vec4) { 1.0f, 1.0f, 1.0f, 1.0f}
#else
#define UI_BACKGROUND_COLOR (vec4) {45/255.0f, 45/255.0f, 48/255.0f, 255/255.0f}
#define UI_FILE_SWITCH_AREA_ITEM_BACKGROUND (vec4) {0/255.0f, 122/255.0f, 204/255.0f, 255/255.0f}
#define UI_FILE_SWITCH_AREA_SELECTED_ITEM_BACKGROUND (vec4) {33/255.0f, 122/255.0f, 204/255.0f, 255/255.0f}
#define UI_TOP_MENU_TEXT_COLOR (vec4) {255/255.0f, 255/255.0f, 255/255.0f, 255/255.0f}
#define UI_TEXT_AREA_COLOR (vec4) {30/255.0f, 30/255.0f, 30/255.0f, 255/255.0f}
#define UI_TOP_MENU_SELECTION_COLOR (vec4) {20/255.0f, 20/255.0f, 20/255.0f, 255/255.0f}
#define UI_SUB_MENU_SELECTION_COLOR (vec4) {0/255.0f, 0/255.0f, 255/255.0f, 255/255.0f}
#define UI_FILE_SWITCH_AREA_TEXT_COLOR (vec4) {255/255.0f, 255/255.0f, 255/255.0f, 255/255.0f}
#define UI_MAIN_TEXT_COLOR (vec4) { 0.9f, 0.9f, 0.9f, 1.0f }
#define UI_MAIN_TEXT_CURSOR_COLOR (vec4) { 0.9f, 0.9f, 0.9f, 1.0f }
#define UI_MAIN_TEXT_LINE_NUMBER_COLOR (vec4) { 0.9f, 0.9f, 0.9f, 1.0f }
#define UI_CONSOLE_VIEW_TEXT_COLOR (vec4) { 0.0f, 0.9f, 0.0f, 1.0f }
#define UI_CONSOLE_VIEW_CURSOR_COLOR (vec4) { 1.0f, 1.0f, 1.0f, 0.5f }
#define UI_CONSOLE_VIEW_BACKGROUND_COLOR (vec4) { 0.0f, 0.0f, 0.0f, 1.0f}
#define UI_CONSOLE_INPUT_TEXT_COLOR (vec4) { 0.0f, 0.9f, 0.0f, 1.0f }
#define UI_CONSOLE_INPUT_CURSOR_COLOR (vec4) { 1.0f, 1.0f, 1.0f, 0.5f }
#define UI_CONSOLE_INPUT_BACKGROUND_COLOR (vec4) { 0.1f, 0.0f, 0.0f, 1.0f}
#endif

#define UI_TITLE_TEXT_COLOR (vec4) {153/255.0f, 153/255.0f, 153/255.0f, 255/255.0f}
#define UI_DIALOG_VIEW_FONT_COLOR (vec4) { 0.1f, 0.1f, 0.1f, 1.0f }
#define UI_DIALOG_VIEW_CURSOR_COLOR (vec4) { 0.1f, 0.1f, 0.1f, 0.5f }
#define UI_DIALOG_VIEW_BACKGROUND_COLOR (vec4) { 0.7f, 0.7f, 0.7f, 1.0f}
#define UI_DIALOG_INPUT_FONT_COLOR (vec4) { 0.1f, 0.1f, 0.1f, 1.0f }
#define UI_DIALOG_INPUT_CURSOR_COLOR (vec4) { 0.1f, 0.1f, 0.1f, 0.5f }
#define UI_DIALOG_INPUT_BACKGROUND_COLOR (vec4) { 1.0f, 1.0f, 1.0f, 1.0f}
#define UI_SEARCH_INPUT_TEXT_COLOR (vec4) { 0.0f, 0.9f, 0.0f, 1.0f }
#define UI_SEARCH_INPUT_CURSOR_COLOR (vec4) { 1.0f, 1.0f, 1.0f, 0.5f }
#define UI_SEARCH_INPUT_BACKGROUND_COLOR (vec4) { 0.3f, 0.3f, 0.0f, 1.0f}
#define UI_SEARCH_VIEW_TEXT_COLOR (vec4) { 0.0f, 0.9f, 0.0f, 1.0f }
#define UI_SEARCH_VIEW_CURSOR_COLOR (vec4) { 1.0f, 1.0f, 1.0f, 0.5f }
#define UI_SEARCH_VIEW_BACKGROUND_COLOR (vec4) { 0.3f, 0.3f, 0.0f, 1.0f}

#define UI_RED_COLOR (vec4) {1.0f, 0.0f, 0.0f, 1.0f}
#define UI_GREEN_COLOR (vec4) {0.0f, 1.0f, 0.0f, 1.0f}
#define UI_BLUE_COLOR (vec4) {0.0f, 0.0f, 1.0f, 1.0f}
#define UI_WHITE_COLOR (vec4) {1.0f, 1.0f, 1.0f, 1.0f}

// @temporary
#define UI_FILE_SWITCH_AREA_ITEM_BACKGROUND_LEVEL2 (vec4) {200/255.0f, 155.0f/255.0f, 155.0f/255.0f, 255/255.0f}
#define UI_FILE_SWITCH_AREA_SELECTED_ITEM_BACKGROUND_LEVEL2 (vec4) {255.0f/255.0f, 50.0f/255.0f, 50.0f/255.0f, 255/255.0f}

#endif