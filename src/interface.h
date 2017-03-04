#ifndef HOHEX_INTERFACE_H
#define HOHEX_INTERFACE_H
#include "common.h"
#include "math/homath.h"
#include "editor.h"

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

#endif
