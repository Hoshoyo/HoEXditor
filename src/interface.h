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
  vec4 color;
  float render_width_pos;
  float render_height_pos;
  float mouse_width_min;
  float mouse_width_max;
  float mouse_height_min;
  float mouse_height_max;
  interface_top_menu_item* next;
};

void render_interface();

void init_interface();
void destroy_interface();
void ui_update_text_container_paddings(Text_Container* container);

internal void render_top_header();
internal void render_top_menu();
internal void render_text_area();
internal void render_file_switch_area();
internal void render_left_column();
internal void render_right_column();
internal void render_footer();

internal void prerender_top_menu();
internal void render_submenus();
internal void add_top_menu_item(u8* name,
  vec4 color,
  float render_width_pos,
  float render_height_pos,
  float mouse_width_min,
  float mouse_width_max,
  float mouse_height_min,
  float mouse_height_max);

#endif
