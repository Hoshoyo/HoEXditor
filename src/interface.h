#ifndef HOHEX_INTERFACE_H
#define HOHEX_INTERFACE_H
#include "common.h"
#include "math/homath.h"
#include "editor.h"

typedef struct interface_top_menu_item_struct interface_top_menu_item;

struct interface_top_menu_item_struct
{
  u8* name;
  u32 name_size;
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

internal void prerender_top_menu();
internal interface_top_menu_item* add_top_menu_item(interface_top_menu_item* root,
  u8* name,
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
