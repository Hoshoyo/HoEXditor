#ifndef HOHEX_INTERFACE_H
#define HOHEX_INTERFACE_H
#include "common.h"
#include "math/homath.h"
#include "editor.h"

typedef struct interface_top_menu_item_struct interface_top_menu_item;
typedef struct interface_top_menu_item_id_struct interface_top_menu_item_id;
typedef struct interface_size_struct interface_size;

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
  T_UI_SUBMENU_ITEM_3_1,
  T_UI_SUBMENU_ITEM_3_2,
  T_UI_SUBMENU_ITEM_4_1,
};

struct interface_top_menu_item_id_struct
{
  u8* name;
  enum interface_sub_menu_item_type type;
};

void render_interface();

void init_interface();
void destroy_interface();
void ui_update_text_container_paddings(Text_Container* container);

internal void render_top_header();
void render_top_menu();
internal void render_top_menu_container();
internal void render_top_menu_items(interface_top_menu_item* top_menu_item);
internal void render_text_area();
internal void render_file_switch_area();
internal void render_left_column();
internal void render_right_column();
internal void render_footer();
internal interface_size get_submenu_bounds(interface_top_menu_item_id* top_menu_item_list, s32 list_size, float height_per_item);

void prerender_top_menu();
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
