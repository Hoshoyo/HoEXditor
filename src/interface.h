#ifndef HOHEX_INTERFACE_H
#define HOHEX_INTERFACE_H
#include "common.h"
#include "math/homath.h"
#include "editor.h"

extern bool is_interface_initialized;

typedef struct interface_top_menu_item_struct interface_top_menu_item;
typedef struct interface_top_menu_item_id_struct interface_top_menu_item_id;
typedef struct interface_size_struct interface_size;
typedef struct interface_panel_struct interface_panel;
typedef struct ui_dialog_struct ui_dialog;

struct interface_panel_struct
{
  Editor_State* es;
  u32 x;
  u32 y;
  u32 width;
  u32 height;
  vec4 background_color;

  bool visible;

  // main_text_panel info
  bool is_main_text_panel;
  s32 main_text_panel_vertical_level;

  // Used when interface_panel is inside a list of panels.
  interface_panel* next;
  interface_panel* previous;
};


struct interface_size_struct
{
  float width;
  float height;
};

struct interface_top_menu_item_struct
{
  u8* name;
  u32 name_size;
  enum interface_sub_menu_item_type code;
  vec4 text_color;
  vec4 selection_color;
  float render_width_pos;
  float render_height_pos;
  float mouse_width_min;
  float mouse_width_max;
  float mouse_height_min;
  float mouse_height_max;
  bool has_sub_container;
  bool is_sub_container_open;
  float sub_container_width_min;
  float sub_container_width_max;
  float sub_container_height_min;
  float sub_container_height_max;
  interface_top_menu_item* parent;
  interface_top_menu_item* items;
  interface_top_menu_item* next;
};

enum interface_sub_menu_item_type
{
  T_UI_MENU_ITEM_1,
  T_UI_MENU_ITEM_2,
  T_UI_MENU_ITEM_3,
  T_UI_MENU_ITEM_4,
  T_UI_SUBMENU_ITEM_1_1,
  T_UI_SUBMENU_ITEM_1_2,
  T_UI_SUBMENU_ITEM_1_3,
  T_UI_SUBMENU_ITEM_1_4,
  T_UI_SUBMENU_ITEM_1_5,
  T_UI_SUBMENU_ITEM_1_6,
  T_UI_SUBMENU_ITEM_1_7,
  T_UI_SUBMENU_ITEM_1_8,
  T_UI_SUBMENU_ITEM_2_1,
  T_UI_SUBMENU_ITEM_2_2,
  T_UI_SUBMENU_ITEM_2_3,
  T_UI_SUBMENU_ITEM_2_4,
  T_UI_SUBMENU_ITEM_2_5,
  T_UI_SUBMENU_ITEM_2_6,
  T_UI_SUBMENU_ITEM_2_7,
  T_UI_SUBMENU_ITEM_2_8,
  T_UI_SUBMENU_ITEM_2_9,
  T_UI_SUBMENU_ITEM_3_1,
  T_UI_SUBMENU_ITEM_3_2,
  T_UI_SUBMENU_ITEM_3_3,
  T_UI_SUBMENU_ITEM_3_4,
  T_UI_SUBMENU_ITEM_3_5,
  T_UI_SUBMENU_ITEM_3_6,
  T_UI_SUBMENU_ITEM_3_7,
  T_UI_SUBMENU_ITEM_3_8,
  T_UI_SUBMENU_ITEM_4_1,
};

struct interface_top_menu_item_id_struct
{
  u8* name;
  enum interface_sub_menu_item_type type;
};

struct ui_dialog_struct
{
	float x;
	float y;
	float width;
	float height;
	float ratio;
	bool dark_background_when_rendering;

	void(*dialog_callback)(u8*);

	interface_panel* view_panel;
	interface_panel* input_panel;
};

void init_interface();
void destroy_interface();
void render_interface();
void render_interface_panel(interface_panel* panel);
Editor_State* ui_get_focused_editor();
void ui_handle_mouse_click(s32 x, s32 y);
void ui_handle_key_down(s32 key);
s32 ui_open_file(bool empty, u8* file_path);
s32 ui_save_file(u8* file_path);
s32 ui_close_file();
s32 ui_close_all_files();
s32 ui_search_word(u8* word, s64 word_length);
s32 ui_split_view();
s32 ui_clone_and_split_view();
void ui_handle_file_drop(u8* path, s32 x, s32 y);
bool ui_is_console_window_visible();
bool ui_is_search_window_visible();
void ui_change_search_window_visibility(bool visible);
void ui_change_console_window_visibility(bool visible);
void ui_change_search_window_visibility(bool visible);

// Dialogs
void ui_show_open_file_dialog();
void ui_show_save_file_dialog();

void prerender_top_menu();
void render_top_menu();

void free_interface_panel(interface_panel* interface_panel);

internal float get_main_panel_width_per_level(s32 vertical_level);
internal s32 get_number_of_main_text_vertical_levels();
internal interface_panel* get_first_main_text_panel(s32 vertical_level);
internal interface_panel* get_on_screen_main_text_panel(s32 vertical_level);
internal interface_panel* get_next_main_text_panel_by_level(interface_panel* reference_panel);
// returns new interface_panel created
internal interface_panel* insert_main_text_window(bool empty, u8* filename);
internal interface_panel* insert_clone_main_text_window(text_id tid);
// returns the interface panel that should assume screen
internal interface_panel* remove_main_text_window(interface_panel* main_text_window);
// destroy panel and refresh the panel that is on screen and focused editor state
internal s32 close_panel(interface_panel* panel);
internal s32 close_file(text_id tid);
internal void init_console_window();
internal void init_search_window();
internal s32 search_char_handler(Editor_State* es, s32 key);
internal void add_auxiliar_panel(interface_panel* auxiliar_panel);

internal void update_panels_bounds();
internal void render_panels();
internal void handle_top_menu_click(interface_top_menu_item* top_menu_item, s32 x, s32 y);
internal void update_interface_panel(interface_panel* panel);
internal void change_focused_editor(Editor_State* es);
internal void change_main_text_panel_on_screen(interface_panel* panel, s32 vertical_level);
internal void render_top_header();

internal void render_top_menu_container();
internal void render_top_menu_items(interface_top_menu_item* top_menu_item);
internal void render_text_area();
internal void render_file_switch_area();
internal void render_left_column();
internal void render_right_column();
internal void render_footer();
internal interface_size get_submenu_bounds(interface_top_menu_item_id* top_menu_item_list, s32 list_size, float height_per_item);

internal void open_dialog(ui_dialog* dialog);
internal void close_dialog(ui_dialog* dialog);
internal void update_active_dialog();
internal void init_open_file_dialog();
internal void init_save_file_dialog();
internal void open_file_dialog_callback(u8* text);
internal void save_file_dialog_callback(u8* text);

internal void destroy_top_menu_prerender();
internal interface_top_menu_item* add_top_menu_item(interface_top_menu_item** root,
  u8* name,
  enum interface_sub_menu_item_type code,
  vec4 text_color,
  vec4 selection_color,
  float render_width_pos,
  float render_height_pos,
  float mouse_width_min,
  float mouse_width_max,
  float mouse_height_min,
  float mouse_height_max,
  bool has_sub_container,
  float sub_container_width_min,
  float sub_container_width_max,
  float sub_container_height_min,
  float sub_container_height_max,
  interface_top_menu_item* items);

#endif
