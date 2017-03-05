#include "interface.h"
#include "os_dependent.h"
#include "font_rendering.h"
#include "memory.h"

extern Window_State win_state;
extern u8* _tm_file_name;

Font_Rendering* fd;

#define UI_ICON_PATH "./res/icon.png"
GLuint ui_icon_texture_id;

#define UI_TITLE "HoEXditor - Your hexadecimal editor."
#define UI_MENU_ITEM_1 "File"
#define UI_MENU_ITEM_2 "Edit"
#define UI_MENU_ITEM_3 "View"
#define UI_MENU_ITEM_4 "Help"

#define UI_TOP_HEADER_HEIGHT 0.0f
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
#define UI_FILE_SWITCH_AREA_TEXT_COLOR (vec4) {255/255.0f, 255/255.0f, 255/255.0f, 255/255.0f}
#define UI_FILE_SWITCH_AREA_ITEM_BACKGROUND (vec4) {0/255.0f, 122/255.0f, 204/255.0f, 255/255.0f}
#define UI_RED_COLOR (vec4) {1.0f, 0.0f, 0.0f, 1.0f}
#define UI_GREEN_COLOR (vec4) {0.0f, 1.0f, 0.0f, 1.0f}
#define UI_BLUE_COLOR (vec4) {0.0f, 0.0f, 1.0f, 1.0f}
#define UI_WHITE_COLOR (vec4) {1.0f, 1.0f, 1.0f, 1.0f}

void init_interface()
{
  s32 width, height, channels;
  u8* data = create_texture(UI_ICON_PATH, &width, &height, &channels);
	ui_icon_texture_id = gen_gl_texture(data, width, height);
  free_texture(data);

  fd = halloc(sizeof(Font_Rendering));
  load_font("c:/windows/fonts/consola.ttf", 16, &fd);
  fill_font(fd, win_state.win_width, win_state.win_height);
}

void destroy_interface()
{
  release_font(&fd);
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
//  render_top_header();
  render_top_menu();
  render_file_switch_area();
  render_text_area();
  render_left_column();
  render_right_column();
  render_footer();
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
  const float top_menu_item_initial_width_spacement = 20.0f;
  const float top_menu_item_width_spacement = 10.0f;
  float top_menu_min_height = win_state.win_height - UI_TOP_HEADER_HEIGHT - UI_TOP_MENU_HEIGHT;
  float top_menu_max_height = win_state.win_height - UI_TOP_HEADER_HEIGHT;
  float top_menu_min_width = 0;
  float top_menu_max_width = win_state.win_width;
  float top_menu_item_height_spacement = round((top_menu_max_height - top_menu_min_height - fd->max_height)/2.0f);
  vec4 top_menu_color = UI_BACKGROUND_COLOR;
  render_transparent_quad(top_menu_min_width,
    top_menu_min_height,
    top_menu_max_width,
    top_menu_max_height,
    &top_menu_color);
  vec4 top_menu_text_color = UI_TOP_MENU_TEXT_COLOR;

  Font_RenderInInfo font_in_info = {0};
  Font_RenderOutInfo font_out_info;

  render_text(top_menu_min_width + top_menu_item_initial_width_spacement,
    top_menu_min_height + top_menu_item_height_spacement,
    UI_MENU_ITEM_1,
    sizeof(UI_MENU_ITEM_1) - 1,
    &top_menu_text_color);

  prerender_text(top_menu_min_width + top_menu_item_initial_width_spacement,
    top_menu_min_height + top_menu_item_height_spacement,
    UI_MENU_ITEM_2,
    sizeof(UI_MENU_ITEM_2) - 1,
    &font_out_info,
    &font_in_info);

  render_text(font_out_info.exit_width + top_menu_item_width_spacement,
    top_menu_min_height + top_menu_item_height_spacement,
    UI_MENU_ITEM_2,
    sizeof(UI_MENU_ITEM_2) - 1,
    &top_menu_text_color);

  prerender_text(font_out_info.exit_width + top_menu_item_width_spacement,
    top_menu_min_height + top_menu_item_height_spacement,
    UI_MENU_ITEM_3,
    sizeof(UI_MENU_ITEM_3) - 1,
    &font_out_info,
    &font_in_info);

  render_text(font_out_info.exit_width + top_menu_item_width_spacement,
    top_menu_min_height + top_menu_item_height_spacement,
    UI_MENU_ITEM_3,
    sizeof(UI_MENU_ITEM_3) - 1,
    &top_menu_text_color);

  prerender_text(font_out_info.exit_width + top_menu_item_width_spacement,
    top_menu_min_height + top_menu_item_height_spacement,
    UI_MENU_ITEM_4,
    sizeof(UI_MENU_ITEM_4) - 1,
    &font_out_info,
    &font_in_info);

  render_text(font_out_info.exit_width + top_menu_item_width_spacement,
    top_menu_min_height + top_menu_item_height_spacement,
    UI_MENU_ITEM_4,
    sizeof(UI_MENU_ITEM_4) - 1,
    &top_menu_text_color);
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
