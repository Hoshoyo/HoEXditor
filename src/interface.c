#include "interface.h"
#include "os_dependent.h"
#include "font_rendering.h"
#include "memory.h"
#include "input.h"

extern Window_State win_state;
extern u8* _tm_file_name;

Font_Rendering* fd;
interface_top_menu_item* _if_top_menu_items = null;
bool is_interface_initialized = false;

#define MOD(n) (n) > 0 ? (n) : -(n)

#define UI_ICON_PATH "./res/icon.png"
GLuint ui_icon_texture_id;

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

#define UI_SUBMENU_ITEM_4_1 "About"

#define UI_TOP_HEADER_HEIGHT 35.0f
#define UI_TOP_MENU_HEIGHT 25.0f
#define UI_FILE_SWITCH_AREA_HEIGHT 28.0f
#define UI_LEFT_COLUMN_WIDTH 2.0f
#define UI_RIGHT_COLUMN_WIDTH 2.0f
#define UI_FOOTER_HEIGHT 2.0f
#define UI_TEXT_PADDING 10.0f

#define UI_BACKGROUND_COLOR (vec4) {45/255.0f, 45/255.0f, 48/255.0f, 255/255.0f}
#define UI_TEXT_AREA_COLOR (vec4) {30/255.0f, 30/255.0f, 30/255.0f, 255/255.0f}
#define UI_TITLE_TEXT_COLOR (vec4) {153/255.0f, 153/255.0f, 153/255.0f, 255/255.0f}
#define UI_TOP_MENU_TEXT_COLOR (vec4) {255/255.0f, 255/255.0f, 255/255.0f, 255/255.0f}
#define UI_TOP_MENU_SELECTION_COLOR (vec4) {20/255.0f, 20/255.0f, 20/255.0f, 255/255.0f}
#define UI_SUB_MENU_SELECTION_COLOR (vec4) {0/255.0f, 0/255.0f, 255/255.0f, 255/255.0f}
#define UI_FILE_SWITCH_AREA_TEXT_COLOR (vec4) {255/255.0f, 255/255.0f, 255/255.0f, 255/255.0f}
#define UI_FILE_SWITCH_AREA_ITEM_BACKGROUND (vec4) {0/255.0f, 122/255.0f, 204/255.0f, 255/255.0f}
#define UI_RED_COLOR (vec4) {1.0f, 0.0f, 0.0f, 1.0f}
#define UI_GREEN_COLOR (vec4) {0.0f, 1.0f, 0.0f, 1.0f}
#define UI_BLUE_COLOR (vec4) {0.0f, 0.0f, 1.0f, 1.0f}
#define UI_WHITE_COLOR (vec4) {1.0f, 1.0f, 1.0f, 1.0f}

void init_interface()
{
  s32 width, height, channels;
  is_interface_initialized = true;
  u8* data = create_texture(UI_ICON_PATH, &width, &height, &channels);
	ui_icon_texture_id = gen_gl_texture(data, width, height);
  free_texture(data);

  fd = halloc(sizeof(Font_Rendering));
  load_font("c:/windows/fonts/consola.ttf", 16, &fd);
  fill_font(fd, win_state.win_width, win_state.win_height);

  prerender_top_menu();
}

void destroy_interface()
{
  is_interface_initialized = false;
  destroy_top_menu_prerender();
  release_font(&fd);
}

void destroy_top_menu_prerender()
{
  interface_top_menu_item* top_menu_item = _if_top_menu_items;

  while (top_menu_item != null)
  {
    interface_top_menu_item* aux = top_menu_item;
    top_menu_item = top_menu_item->next;

    hfree(aux->name);
    hfree(aux);
  }

  _if_top_menu_items = null;
}

void ui_update_text_container_paddings(Text_Container* container)
{
  container->left_padding = UI_LEFT_COLUMN_WIDTH + UI_TEXT_PADDING;
  container->right_padding = UI_RIGHT_COLUMN_WIDTH + UI_TEXT_PADDING;
  container->top_padding = UI_TOP_HEADER_HEIGHT + UI_TOP_MENU_HEIGHT + UI_FILE_SWITCH_AREA_HEIGHT + UI_TEXT_PADDING;
	container->bottom_padding = UI_FOOTER_HEIGHT + UI_TEXT_PADDING;
}

void render_interface()
{
  Font_Rendering* previous_font = font_rendering;
  bind_font(&fd);
  update_font((float)win_state.win_width, (float)win_state.win_height);
  render_top_header();
  render_file_switch_area();
  render_text_area();
  render_left_column();
  render_right_column();
  render_footer();

  //render_top_menu();

  bind_font(&previous_font);
}

void render_top_header()
{
  const vec2 icon_x_position = (vec2) {15.0f, 35.0f};
  const vec2 icon_y_position = (vec2) {10.0f, 30.0f};
  const float title_x_position = 45.0f;
  const float title_y_position = 12.0f;
  float top_header_min_height = win_state.win_height - UI_TOP_HEADER_HEIGHT;
  float top_header_max_height = win_state.win_height;
  float top_header_min_width = 0;
  float top_header_max_width = win_state.win_width;
  vec4 top_header_color = UI_BACKGROUND_COLOR;
  render_transparent_quad(top_header_min_width, top_header_min_height, top_header_max_width, top_header_max_height, &top_header_color);
  stbi_set_flip_vertically_on_load(1);
  render_textured_quad(top_header_min_width + icon_x_position.x,
    top_header_min_height + icon_y_position.x,
    top_header_min_width + icon_x_position.y,
    top_header_min_height + icon_y_position.y,
    ui_icon_texture_id);
  vec4 top_header_text_color = UI_TITLE_TEXT_COLOR;
  render_text(top_header_min_width + title_x_position,
    top_header_min_height + title_y_position,
    UI_TITLE,
    sizeof(UI_TITLE) - 1,
    &top_header_text_color);
}

void render_top_menu()
{
  render_top_menu_container();
  render_top_menu_items(_if_top_menu_items);
}

void render_top_menu_container()
{
  float top_menu_min_height = win_state.win_height - UI_TOP_HEADER_HEIGHT - UI_TOP_MENU_HEIGHT;
  float top_menu_max_height = win_state.win_height - UI_TOP_HEADER_HEIGHT;
  float top_menu_min_width = 0;
  float top_menu_max_width = win_state.win_width;
  vec4 top_menu_color = UI_BACKGROUND_COLOR;
  render_transparent_quad(top_menu_min_width,
    top_menu_min_height,
    top_menu_max_width,
    top_menu_max_height,
    &top_menu_color);
}

void handle_top_menu_click(interface_top_menu_item* top_menu_item, s32 x, s32 y)
{
  s32 mouse_x = x;
  s32 mouse_y = win_state.win_height - y;

  if (top_menu_item == null) top_menu_item = _if_top_menu_items;

  while (top_menu_item != null)
  {
    if (mouse_x >= top_menu_item->mouse_width_min &&
      mouse_x <= top_menu_item->mouse_width_max &&
      mouse_y >= top_menu_item->mouse_height_min &&
      mouse_y <= top_menu_item->mouse_height_max)
    {
      handle_top_menu_event(top_menu_item->code);
    }

    if (top_menu_item->has_sub_container && top_menu_item->is_sub_container_open)
      handle_top_menu_click(top_menu_item->items, x, y);

    top_menu_item = top_menu_item->next;
  }
}

void render_top_menu_items(interface_top_menu_item* top_menu_item)
{
  s32 mouse_x = mouse_state.x;
  s32 mouse_y = win_state.win_height - mouse_state.y;

  while (top_menu_item != null)
  {
    if (
      (mouse_x >= top_menu_item->mouse_width_min &&
      mouse_x <= top_menu_item->mouse_width_max &&
      mouse_y >= top_menu_item->mouse_height_min &&
      mouse_y <= top_menu_item->mouse_height_max) ||
      (top_menu_item->is_sub_container_open &&
      mouse_x >= top_menu_item->sub_container_width_min &&
      mouse_x <= top_menu_item->sub_container_width_max &&
      mouse_y >= top_menu_item->sub_container_height_min &&
      mouse_y <= top_menu_item->sub_container_height_max)
    )
    {
      render_transparent_quad(top_menu_item->mouse_width_min,
        top_menu_item->mouse_height_min,
        top_menu_item->mouse_width_max,
        top_menu_item->mouse_height_max,
        &top_menu_item->selection_color);

      if (top_menu_item->has_sub_container)
      {
        render_transparent_quad(top_menu_item->sub_container_width_min,
          top_menu_item->sub_container_height_min,
          top_menu_item->sub_container_width_max,
          top_menu_item->sub_container_height_max,
          &top_menu_item->selection_color);

        interface_top_menu_item* top_submenu_item = top_menu_item->items;

        while (top_submenu_item != null)
        {
          render_text(top_submenu_item->render_width_pos,
            top_submenu_item->render_height_pos,
            top_submenu_item->name,
            top_submenu_item->name_size,
            &top_submenu_item->text_color);

          top_submenu_item = top_submenu_item->next;
        }

        render_top_menu_items(top_menu_item->items);
      }
      top_menu_item->is_sub_container_open = true;
    }
    else
      top_menu_item->is_sub_container_open = false;

    render_text(top_menu_item->render_width_pos,
      top_menu_item->render_height_pos,
      top_menu_item->name,
      top_menu_item->name_size,
      &top_menu_item->text_color);

    top_menu_item = top_menu_item->next;
  }
}

interface_size get_submenu_bounds(interface_top_menu_item_id* top_menu_item_list, s32 list_size, float height_per_item)
{
  u32 aux;
  interface_size size = {.width = 0, .height = 0};
  Font_RenderInInfo font_in = {0};
  Font_RenderOutInfo font_out;

  for (aux=0; aux<list_size; aux++)
  {
    prerender_text(0, 0, top_menu_item_list[aux].name, hstrlen(top_menu_item_list[aux].name), &font_out, &font_in);
    if (size.width < font_out.exit_width) size.width = font_out.exit_width;
    size.height += height_per_item;
  }

  return size;
}

void prerender_top_menu()
{
  interface_top_menu_item_id top_menu_items[] = {
    { .name = UI_MENU_ITEM_1, .type = T_UI_MENU_ITEM_1},
    { .name = UI_MENU_ITEM_2, .type = T_UI_MENU_ITEM_2},
    { .name = UI_MENU_ITEM_3, .type = T_UI_MENU_ITEM_3},
    { .name = UI_MENU_ITEM_4, .type = T_UI_MENU_ITEM_4},
  };

  interface_top_menu_item_id sub_menu_items_1[] = {
    { .name = UI_SUBMENU_ITEM_1_1, .type = T_UI_SUBMENU_ITEM_1_1},
    { .name = UI_SUBMENU_ITEM_1_2, .type = T_UI_SUBMENU_ITEM_1_2},
    { .name = UI_SUBMENU_ITEM_1_3, .type = T_UI_SUBMENU_ITEM_1_3},
    { .name = UI_SUBMENU_ITEM_1_4, .type = T_UI_SUBMENU_ITEM_1_4},
    { .name = UI_SUBMENU_ITEM_1_5, .type = T_UI_SUBMENU_ITEM_1_5},
    { .name = UI_SUBMENU_ITEM_1_6, .type = T_UI_SUBMENU_ITEM_1_6},
    { .name = UI_SUBMENU_ITEM_1_7, .type = T_UI_SUBMENU_ITEM_1_7},
    { .name = UI_SUBMENU_ITEM_1_8, .type = T_UI_SUBMENU_ITEM_1_8},
  };

  interface_top_menu_item_id sub_menu_items_2[] = {
    { .name = UI_SUBMENU_ITEM_2_1, .type = T_UI_SUBMENU_ITEM_2_1},
    { .name = UI_SUBMENU_ITEM_2_2, .type = T_UI_SUBMENU_ITEM_2_2},
    { .name = UI_SUBMENU_ITEM_2_3, .type = T_UI_SUBMENU_ITEM_2_3},
    { .name = UI_SUBMENU_ITEM_2_4, .type = T_UI_SUBMENU_ITEM_2_4},
    { .name = UI_SUBMENU_ITEM_2_5, .type = T_UI_SUBMENU_ITEM_2_5},
    { .name = UI_SUBMENU_ITEM_2_6, .type = T_UI_SUBMENU_ITEM_2_6},
    { .name = UI_SUBMENU_ITEM_2_7, .type = T_UI_SUBMENU_ITEM_2_7},
    { .name = UI_SUBMENU_ITEM_2_8, .type = T_UI_SUBMENU_ITEM_2_8},
    { .name = UI_SUBMENU_ITEM_2_9, .type = T_UI_SUBMENU_ITEM_2_9},
  };

  interface_top_menu_item_id sub_menu_items_3[] = {
    { .name = UI_SUBMENU_ITEM_3_1, .type = T_UI_SUBMENU_ITEM_3_1},
    { .name = UI_SUBMENU_ITEM_3_2, .type = T_UI_SUBMENU_ITEM_3_2},
    { .name = UI_SUBMENU_ITEM_3_3, .type = T_UI_SUBMENU_ITEM_3_3},
    { .name = UI_SUBMENU_ITEM_3_4, .type = T_UI_SUBMENU_ITEM_3_4},
    { .name = UI_SUBMENU_ITEM_3_5, .type = T_UI_SUBMENU_ITEM_3_5},
  };

  interface_top_menu_item_id sub_menu_items_4[] = {
    { .name = UI_SUBMENU_ITEM_4_1, .type = T_UI_SUBMENU_ITEM_4_1},
  };

  u32 aux;
  interface_top_menu_item* submenu;
  const float top_menu_item_initial_width_spacement = 20.0f;
  const float top_menu_item_width_spacement = 5.0f;
  const float top_submenu_item_initial_height_spacement = 20.0f;  // TODO: MAX_HEIGHT + DESCENT + SPACEMENT
  float top_menu_previous_width;
  float descent_mod = MOD(fd->descent);
  float sub_menu_previous_height;
  float top_menu_min_height = win_state.win_height - UI_TOP_HEADER_HEIGHT - UI_TOP_MENU_HEIGHT;
  float top_menu_max_height = win_state.win_height - UI_TOP_HEADER_HEIGHT;
  float top_menu_min_width = 0;
  float top_menu_max_width = win_state.win_width;
  float top_menu_item_height_spacement = round((top_menu_max_height - top_menu_min_height - fd->max_height)/2.0f);
  vec4 top_menu_color = UI_BACKGROUND_COLOR;
  vec4 top_menu_text_color = UI_TOP_MENU_TEXT_COLOR;
  vec4 top_menu_selection_color = UI_TOP_MENU_SELECTION_COLOR;
  vec4 sub_menu_selection_color = UI_SUB_MENU_SELECTION_COLOR;
  s32 submenu_items_size;
  interface_size bounds;
  Font_RenderInInfo font_in_info = {0};
  Font_RenderOutInfo font_out_info;

  destroy_top_menu_prerender();

  render_transparent_quad(top_menu_min_width,
    top_menu_min_height,
    top_menu_max_width,
    top_menu_max_height,
    &top_menu_color);

  /* MENU ITEM 1 */

  prerender_text(top_menu_min_width + top_menu_item_initial_width_spacement,
    top_menu_min_height + top_menu_item_height_spacement,
    UI_MENU_ITEM_1,
    sizeof(UI_MENU_ITEM_1) - 1,
    &font_out_info,
    &font_in_info);

  sub_menu_previous_height = top_menu_min_height + top_menu_item_height_spacement;
  submenu = null;
  submenu_items_size = sizeof(sub_menu_items_1)/sizeof(interface_top_menu_item_id);
  bounds = get_submenu_bounds(sub_menu_items_1, submenu_items_size, top_submenu_item_initial_height_spacement);

  for (u32 aux=0; aux<submenu_items_size; ++aux)
  {
      add_top_menu_item(&submenu,
      sub_menu_items_1[aux].name,
      sub_menu_items_1[aux].type,
      top_menu_text_color,
      sub_menu_selection_color,
      top_menu_min_width + top_menu_item_initial_width_spacement,
      sub_menu_previous_height - top_submenu_item_initial_height_spacement,
      top_menu_min_width + top_menu_item_initial_width_spacement - top_menu_item_width_spacement,
      top_menu_min_width + top_menu_item_initial_width_spacement + top_menu_item_width_spacement + bounds.width,
      sub_menu_previous_height - top_submenu_item_initial_height_spacement + fd->descent,
      sub_menu_previous_height - top_submenu_item_initial_height_spacement + fd->max_height,
      false, -1, -1, -1, -1, null);

      sub_menu_previous_height -= top_submenu_item_initial_height_spacement;
  }

  add_top_menu_item(&_if_top_menu_items,
    UI_MENU_ITEM_1,
    T_UI_MENU_ITEM_1,
    top_menu_text_color,
    top_menu_selection_color,
    top_menu_min_width + top_menu_item_initial_width_spacement,
    top_menu_min_height + top_menu_item_height_spacement,
    top_menu_min_width + top_menu_item_initial_width_spacement - top_menu_item_width_spacement,
    font_out_info.exit_width + top_menu_item_width_spacement,
    top_menu_min_height + top_menu_item_height_spacement + fd->descent,
    top_menu_min_height + top_menu_item_height_spacement + fd->max_height,
    true,
    top_menu_min_width + top_menu_item_initial_width_spacement - top_menu_item_width_spacement,
    top_menu_min_width + top_menu_item_initial_width_spacement + top_menu_item_width_spacement + bounds.width,
    top_menu_min_height + top_menu_item_height_spacement + fd->descent - bounds.height,
    top_menu_min_height + top_menu_item_height_spacement + fd->descent,
    submenu);

  top_menu_previous_width = font_out_info.exit_width;

  /* MENU ITEM 2 */

  prerender_text(font_out_info.exit_width + 2 * top_menu_item_width_spacement,
    top_menu_min_height + top_menu_item_height_spacement,
    UI_MENU_ITEM_2,
    sizeof(UI_MENU_ITEM_2) - 1,
    &font_out_info,
    &font_in_info);

  sub_menu_previous_height = top_menu_min_height + top_menu_item_height_spacement;
  submenu = null;
  submenu_items_size = sizeof(sub_menu_items_2)/sizeof(interface_top_menu_item_id);
  bounds = get_submenu_bounds(sub_menu_items_2, submenu_items_size, top_submenu_item_initial_height_spacement);

  for (u32 aux=0; aux<submenu_items_size; ++aux)
  {
    add_top_menu_item(&submenu,
    sub_menu_items_2[aux].name,
    sub_menu_items_2[aux].type,
    top_menu_text_color,
    sub_menu_selection_color,
    top_menu_previous_width + 2 * top_menu_item_width_spacement,
    sub_menu_previous_height - top_submenu_item_initial_height_spacement,
    top_menu_previous_width + top_menu_item_width_spacement,
    top_menu_previous_width + 3 * top_menu_item_width_spacement + bounds.width,
    sub_menu_previous_height - top_submenu_item_initial_height_spacement + fd->descent,
    sub_menu_previous_height - top_submenu_item_initial_height_spacement + fd->max_height,
    false, -1, -1, -1, -1, null);

    sub_menu_previous_height -= top_submenu_item_initial_height_spacement;
  }

  add_top_menu_item(&_if_top_menu_items,
    UI_MENU_ITEM_2,
    T_UI_MENU_ITEM_2,
    top_menu_text_color,
    top_menu_selection_color,
    top_menu_previous_width + 2 * top_menu_item_width_spacement,
    top_menu_min_height + top_menu_item_height_spacement,
    top_menu_previous_width + top_menu_item_width_spacement,
    font_out_info.exit_width + top_menu_item_width_spacement,
    top_menu_min_height + top_menu_item_height_spacement + fd->descent,
    top_menu_min_height + top_menu_item_height_spacement + fd->max_height,
    true,
    top_menu_previous_width + top_menu_item_width_spacement,
    top_menu_previous_width + 3 * top_menu_item_width_spacement + bounds.width,
    top_menu_min_height + top_menu_item_height_spacement + fd->descent - bounds.height,
    top_menu_min_height + top_menu_item_height_spacement + fd->descent,
    submenu);

  top_menu_previous_width = font_out_info.exit_width;

  /* MENU ITEM 3 */

  prerender_text(font_out_info.exit_width + 2 * top_menu_item_width_spacement,
    top_menu_min_height + top_menu_item_height_spacement,
    UI_MENU_ITEM_3,
    sizeof(UI_MENU_ITEM_3) - 1,
    &font_out_info,
    &font_in_info);

  sub_menu_previous_height = top_menu_min_height + top_menu_item_height_spacement;
  submenu = null;
  submenu_items_size = sizeof(sub_menu_items_3)/sizeof(interface_top_menu_item_id);
  bounds = get_submenu_bounds(sub_menu_items_3, submenu_items_size, top_submenu_item_initial_height_spacement);

  for (u32 aux=0; aux<submenu_items_size; ++aux)
  {
    add_top_menu_item(&submenu,
    sub_menu_items_3[aux].name,
    sub_menu_items_3[aux].type,
    top_menu_text_color,
    sub_menu_selection_color,
    top_menu_previous_width + 2 * top_menu_item_width_spacement,
    sub_menu_previous_height - top_submenu_item_initial_height_spacement,
    top_menu_previous_width + top_menu_item_width_spacement,
    top_menu_previous_width + 3 * top_menu_item_width_spacement + bounds.width,
    sub_menu_previous_height - top_submenu_item_initial_height_spacement + fd->descent,
    sub_menu_previous_height - top_submenu_item_initial_height_spacement + fd->max_height,
    false, -1, -1, -1, -1, null);

    sub_menu_previous_height -= top_submenu_item_initial_height_spacement;
  }

  add_top_menu_item(&_if_top_menu_items,
    UI_MENU_ITEM_3,
    T_UI_MENU_ITEM_3,
    top_menu_text_color,
    top_menu_selection_color,
    top_menu_previous_width + 2 * top_menu_item_width_spacement,
    top_menu_min_height + top_menu_item_height_spacement,
    top_menu_previous_width + top_menu_item_width_spacement,
    font_out_info.exit_width + top_menu_item_width_spacement,
    top_menu_min_height + top_menu_item_height_spacement + fd->descent,
    top_menu_min_height + top_menu_item_height_spacement + fd->max_height,
    true,
    top_menu_previous_width + top_menu_item_width_spacement,
    top_menu_previous_width + 3 * top_menu_item_width_spacement + bounds.width,
    top_menu_min_height + top_menu_item_height_spacement + fd->descent - bounds.height,
    top_menu_min_height + top_menu_item_height_spacement + fd->descent,
    submenu);

  top_menu_previous_width = font_out_info.exit_width;

  /* MENU ITEM 4 */

  prerender_text(font_out_info.exit_width + 2 * top_menu_item_width_spacement,
    top_menu_min_height + top_menu_item_height_spacement,
    UI_MENU_ITEM_4,
    sizeof(UI_MENU_ITEM_4) - 1,
    &font_out_info,
    &font_in_info);

  sub_menu_previous_height = top_menu_min_height + top_menu_item_height_spacement;
  submenu = null;
  submenu_items_size = sizeof(sub_menu_items_4)/sizeof(interface_top_menu_item_id);
  bounds = get_submenu_bounds(sub_menu_items_4, submenu_items_size, top_submenu_item_initial_height_spacement);

  for (u32 aux=0; aux<submenu_items_size; ++aux)
  {
    add_top_menu_item(&submenu,
    sub_menu_items_4[aux].name,
    sub_menu_items_4[aux].type,
    top_menu_text_color,
    sub_menu_selection_color,
    top_menu_previous_width + 2 * top_menu_item_width_spacement,
    sub_menu_previous_height - top_submenu_item_initial_height_spacement,
    top_menu_previous_width + top_menu_item_width_spacement,
    top_menu_previous_width + 3 * top_menu_item_width_spacement + bounds.width,
    sub_menu_previous_height - top_submenu_item_initial_height_spacement + fd->descent,
    sub_menu_previous_height - top_submenu_item_initial_height_spacement + fd->max_height,
    false, -1, -1, -1, -1, null);

    sub_menu_previous_height -= top_submenu_item_initial_height_spacement;
  }

  add_top_menu_item(&_if_top_menu_items,
    UI_MENU_ITEM_4,
    T_UI_MENU_ITEM_4,
    top_menu_text_color,
    top_menu_selection_color,
    top_menu_previous_width + 2 * top_menu_item_width_spacement,
    top_menu_min_height + top_menu_item_height_spacement,
    top_menu_previous_width + top_menu_item_width_spacement,
    font_out_info.exit_width + top_menu_item_width_spacement,
    top_menu_min_height + top_menu_item_height_spacement + fd->descent,
    top_menu_min_height + top_menu_item_height_spacement + fd->max_height,
    true,
    top_menu_previous_width + top_menu_item_width_spacement,
    top_menu_previous_width + 3 * top_menu_item_width_spacement + bounds.width,
    top_menu_min_height + top_menu_item_height_spacement + fd->descent - bounds.height,
    top_menu_min_height + top_menu_item_height_spacement + fd->descent,
    submenu);

  top_menu_previous_width = font_out_info.exit_width;
}

interface_top_menu_item* add_top_menu_item(interface_top_menu_item** root,
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
  interface_top_menu_item* items)
{
  interface_top_menu_item* top_menu_item = halloc(sizeof(interface_top_menu_item));

  top_menu_item->name_size = hstrlen(name);
  top_menu_item->name = halloc(top_menu_item->name_size + 1);
  top_menu_item->code = code;
  copy_string(top_menu_item->name, name, top_menu_item->name_size + 1);
  top_menu_item->text_color = text_color;
  top_menu_item->selection_color = selection_color;
  top_menu_item->render_width_pos = render_width_pos;
  top_menu_item->render_height_pos = render_height_pos;
  top_menu_item->mouse_width_min = mouse_width_min;
  top_menu_item->mouse_width_max = mouse_width_max;
  top_menu_item->mouse_height_min = mouse_height_min;
  top_menu_item->mouse_height_max = mouse_height_max;
  top_menu_item->has_sub_container = has_sub_container;
  top_menu_item->is_sub_container_open = false;
  top_menu_item->sub_container_width_min = sub_container_width_min;
  top_menu_item->sub_container_width_max = sub_container_width_max;
  top_menu_item->sub_container_height_min = sub_container_height_min;
  top_menu_item->sub_container_height_max = sub_container_height_max;
  top_menu_item->items = items;
  top_menu_item->next = null;

  if (*root != null)
  {
    interface_top_menu_item* aux = *root;

    while(aux->next != null)
      aux = aux->next;

    aux->next = top_menu_item;
  }
  else
    *root = top_menu_item;

  return top_menu_item;
}

void render_file_switch_area()
{
  const float file_name_width_spacement = 10.0f;
  const float file_name_height_spacement = 2.0f;
  float file_switch_area_min_height = win_state.win_height - UI_TOP_HEADER_HEIGHT - UI_TOP_MENU_HEIGHT - UI_FILE_SWITCH_AREA_HEIGHT;
  float file_switch_area_max_height = win_state.win_height - UI_TOP_HEADER_HEIGHT - UI_TOP_MENU_HEIGHT;
  float file_switch_area_min_width = UI_LEFT_COLUMN_WIDTH;
  float file_switch_area_max_width = win_state.win_width - UI_RIGHT_COLUMN_WIDTH;
  vec4 file_switch_area_color = UI_BACKGROUND_COLOR;
  render_transparent_quad(file_switch_area_min_width, file_switch_area_min_height, file_switch_area_max_width, file_switch_area_max_height, &file_switch_area_color);

  Font_RenderInInfo font_in_info = {0};
  Font_RenderOutInfo font_out_info;
  prerender_text(file_switch_area_min_width + file_name_width_spacement,
    file_switch_area_min_height + file_name_height_spacement,
    _tm_file_name,
    hstrlen(_tm_file_name),
    &font_out_info,
    &font_in_info);

  vec4 file_switch_area_item_color = UI_FILE_SWITCH_AREA_ITEM_BACKGROUND;
  render_transparent_quad(file_switch_area_min_width,
    file_switch_area_min_height,
    font_out_info.exit_width + file_name_width_spacement,
    file_switch_area_min_height + fd->max_height + 2 * file_name_height_spacement,
    &file_switch_area_item_color);

  vec4 file_switch_area_text_color = UI_FILE_SWITCH_AREA_TEXT_COLOR;
  render_text(file_switch_area_min_width + file_name_width_spacement,
    file_switch_area_min_height + file_name_height_spacement,
    _tm_file_name,
    hstrlen(_tm_file_name),
    &file_switch_area_text_color);
}

void render_text_area()
{
  float text_area_min_height = UI_FOOTER_HEIGHT;
  float text_area_max_height = win_state.win_height - UI_TOP_HEADER_HEIGHT - UI_TOP_MENU_HEIGHT - UI_FILE_SWITCH_AREA_HEIGHT;
  float text_area_min_width = UI_LEFT_COLUMN_WIDTH;
  float text_area_max_width = win_state.win_width - UI_RIGHT_COLUMN_WIDTH;
  vec4 text_area_color = UI_TEXT_AREA_COLOR;
  render_transparent_quad(text_area_min_width, text_area_min_height, text_area_max_width, text_area_max_height, &text_area_color);
}

void render_left_column()
{
  float left_column_min_height = UI_FOOTER_HEIGHT;
  float left_column_max_height = win_state.win_height - UI_TOP_HEADER_HEIGHT - UI_TOP_MENU_HEIGHT;
  float left_column_min_width = 0;
  float left_column_max_width = UI_LEFT_COLUMN_WIDTH;
  vec4 left_column_color = UI_BACKGROUND_COLOR;
  render_transparent_quad(left_column_min_width, left_column_min_height, left_column_max_width, left_column_max_height, &left_column_color);
}

void render_right_column()
{
  float right_column_min_height = UI_FOOTER_HEIGHT;
  float right_column_max_height = win_state.win_height - UI_TOP_HEADER_HEIGHT - UI_TOP_MENU_HEIGHT;
  float right_column_min_width = win_state.win_width - UI_RIGHT_COLUMN_WIDTH;
  float right_column_max_width = win_state.win_width;
  vec4 right_column_color = UI_BACKGROUND_COLOR;
  render_transparent_quad(right_column_min_width, right_column_min_height, right_column_max_width, right_column_max_height, &right_column_color);
}

void render_footer()
{
  float footer_min_height = 0;
  float footer_max_height = UI_FOOTER_HEIGHT;
  float footer_min_width = 0;
  float footer_max_width = win_state.win_width;
  vec4 footer_color = UI_BACKGROUND_COLOR;
  render_transparent_quad(footer_min_width, footer_min_height, footer_max_width, footer_max_height, &footer_color);
}
