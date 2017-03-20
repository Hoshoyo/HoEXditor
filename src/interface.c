#include "interface.h"
#include "interface_definitions.h"
#include "os_dependent.h"
#include "font_rendering.h"
#include "text_manager.h"
#include "memory.h"
#include "input.h"

extern Window_State win_state;
extern u8* _tm_block_file_name;
extern void update_container(Editor_State* es);

Font_Rendering* fd;
interface_top_menu_item* _if_top_menu_items = null;
bool is_interface_initialized = false;

Editor_State* focused_editor_state = null;

interface_panel main_text_panel;
Editor_State main_text_es;

interface_panel console_panel;
Editor_State console_es;

#define MOD(n) (n) > 0 ? (n) : -(n)

GLuint ui_icon_texture_id;

#define INIT_TEXT_CONTAINER(Cont, MINX, MAXX, MINY, MAXY, LP, RP, TP, BP) \
Cont.minx = MINX;	\
Cont.maxx = MAXX;	\
Cont.miny = MINY;	\
Cont.maxy = MAXY;	\
Cont.left_padding = LP;	\
Cont.right_padding = RP; \
Cont.top_padding = TP;	\
Cont.bottom_padding = BP	\

void init_interface()
{
  s32 width, height, channels;
  is_interface_initialized = true;

  fd = halloc(sizeof(Font_Rendering));
  load_font("c:/windows/fonts/consola.ttf", 16, &fd);
  fill_font(fd, win_state.win_width, win_state.win_height);
  bind_font(&fd);

  init_font(win_state.win_width, win_state.win_height);  // must be called only one time.

  init_main_text_window();
  change_focused_editor(&main_text_es);

  init_console_window();

  u8* data = create_texture(UI_ICON_PATH, &width, &height, &channels);
	ui_icon_texture_id = gen_gl_texture(data, width, height);
  free_texture(data);

  prerender_top_menu();
}

void init_main_text_window()
{
	load_file(&main_text_es.main_buffer_tid, "./res/empty.txt");

	// init main_text_es
	main_text_es.cursor_info.cursor_offset = 0;
	main_text_es.cursor_info.cursor_column = 0;
	main_text_es.cursor_info.cursor_snaped_column = 0;
	main_text_es.cursor_info.previous_line_count = 0;
	main_text_es.cursor_info.next_line_count = 0;
	main_text_es.cursor_info.this_line_count = 0;
	main_text_es.cursor_info.cursor_line = 0;
	main_text_es.cursor_info.block_offset = 0;
	main_text_es.font_color = UI_MAIN_TEXT_COLOR;
	main_text_es.cursor_color = UI_MAIN_TEXT_CURSOR_COLOR;
  main_text_es.line_number_color = UI_MAIN_TEXT_LINE_NUMBER_COLOR;

	main_text_es.render = true;
	main_text_es.debug = true;
	main_text_es.line_wrap = false;
	main_text_es.mode = EDITOR_MODE_ASCII;
	main_text_es.is_block_text = true;
	main_text_es.render_line_numbers = true;
  main_text_es.show_cursor = true;

	main_text_es.cursor_info.handle_seek = false;

	// @temporary initialization of container for the editor
	INIT_TEXT_CONTAINER(main_text_es.container, 0.0f, 0.0f, 0.0f, 0.0f, 20.0f, 200.0f, 2.0f + fd->max_height, 20.0f);
	//ui_update_text_container_paddings(&main_text_es.container);
	//update_container(&main_text_es);

  setup_view_buffer(&main_text_es, 0, SCREEN_BUFFER_SIZE, true);
	prepare_editor_text(0, BATCH_SIZE);
	prepare_editor_text(1, 1024);

  main_text_panel.es = &main_text_es;
  main_text_panel.x = main_text_es.container.left_padding;
  main_text_panel.y = main_text_es.container.bottom_padding;
  main_text_panel.width = main_text_es.container.right_padding - main_text_es.container.left_padding;
  main_text_panel.height = main_text_es.container.top_padding - main_text_es.container.bottom_padding;
  main_text_panel.background_color = UI_TEXT_AREA_COLOR;
  main_text_panel.visible = true;
  main_text_panel.position = UI_POS_CENTER;
}

void init_console_window()
{
	// init console_es
	console_es.cursor_info.cursor_offset = 0;
	console_es.cursor_info.cursor_column = 0;
	console_es.cursor_info.cursor_snaped_column = 0;
	console_es.cursor_info.previous_line_count = 0;
	console_es.cursor_info.next_line_count = 0;
	console_es.cursor_info.this_line_count = 0;
	console_es.cursor_info.cursor_line = 0;
	console_es.cursor_info.block_offset = 0;
	console_es.font_color = UI_CONSOLE_TEXT_COLOR;
	console_es.cursor_color = UI_CONSOLE_CURSOR_COLOR;
  console_es.line_number_color = (vec4){0, 0, 0, 0};

	console_es.render = true;
	console_es.debug = true;
	console_es.line_wrap = false;
	console_es.mode = EDITOR_MODE_ASCII;
	console_es.is_block_text = false;
	console_es.render_line_numbers = false;
  console_es.show_cursor = false;

	console_es.cursor_info.handle_seek = false;

  console_es.buffer = halloc(sizeof(u8) * UI_CONSOLE_BUFFER_SIZE);
  console_es.buffer_size = UI_CONSOLE_BUFFER_SIZE;
  console_es.buffer_valid_bytes = 0;

	// @temporary initialization of container for the editor
	INIT_TEXT_CONTAINER(console_es.container, 0.0f, 0.0f, 0.0f, 0.0f, 20.0f, 200.0f, 2.0f + fd->max_height, 20.0f);
	//ui_update_text_container_paddings(&main_text_es.container);
	//update_container(&main_text_es);

  //setup_view_buffer(&console_es, 0, SCREEN_BUFFER_SIZE, true);

  console_panel.es = &console_es;
  console_panel.x = console_es.container.left_padding;
  console_panel.y = console_es.container.bottom_padding;
  console_panel.width = console_es.container.right_padding - console_es.container.left_padding;
  console_panel.height = console_es.container.top_padding - console_es.container.bottom_padding;
  console_panel.background_color = UI_CONSOLE_BACKGROUND_COLOR;
  console_panel.visible = true;
  console_panel.position = UI_POS_BOTTOM;
}

void interface_handle_key_down(s32 key)
{
  if (key == VK_F1)
		console_panel.visible = !console_panel.visible;
  if (key == VK_F2)
  {
    if (focused_editor_state == &console_es)
    {
      focused_editor_state->show_cursor = false;
      focused_editor_state = &main_text_es;
      focused_editor_state->show_cursor = true;
    }
    else
    {
      focused_editor_state->show_cursor = false;
      focused_editor_state = &console_es;
      focused_editor_state->show_cursor = true;
    }
  }

}

void render_interface_panel(interface_panel* panel)
{
  vec4 c = UI_RED_COLOR;
  /*render_transparent_quad_with_border(panel->x, panel->y, panel->x + panel->width, panel->y + panel->height, &panel->background_color, &UI_RED_COLOR,
    0,  // top
    0,  // bottom
    0,  // left
    0); // right */

  render_transparent_quad(panel->x, panel->y, panel->x + panel->width, panel->y + panel->height, &panel->background_color);
  render_editor(panel->es);
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

void change_focused_editor(Editor_State* es)
{
	focused_editor_state = es;
}

Editor_State* get_focused_editor()
{
  return focused_editor_state;
}

void update_panels_bounds()
{
  if (console_panel.visible)
  {
    main_text_panel.x = UI_LEFT_COLUMN_WIDTH;
    main_text_panel.y = UI_FOOTER_HEIGHT + UI_CONSOLE_HEIGHT;
    main_text_panel.width = win_state.win_width - UI_RIGHT_COLUMN_WIDTH;
    main_text_panel.height = win_state.win_height - (UI_CONSOLE_HEIGHT + UI_TOP_HEADER_HEIGHT + UI_TOP_MENU_HEIGHT + UI_FILE_SWITCH_AREA_HEIGHT);

    main_text_panel.es->container.left_padding = UI_LEFT_COLUMN_WIDTH + UI_TEXT_PADDING;
    main_text_panel.es->container.right_padding = UI_RIGHT_COLUMN_WIDTH + UI_TEXT_PADDING;
    main_text_panel.es->container.top_padding = UI_TOP_HEADER_HEIGHT + UI_TOP_MENU_HEIGHT + UI_FILE_SWITCH_AREA_HEIGHT + UI_TEXT_PADDING;
    main_text_panel.es->container.bottom_padding = UI_FOOTER_HEIGHT + UI_TEXT_PADDING + UI_CONSOLE_HEIGHT;

    console_panel.x = UI_LEFT_COLUMN_WIDTH;
    console_panel.y = UI_FOOTER_HEIGHT;
    console_panel.width = win_state.win_width - UI_RIGHT_COLUMN_WIDTH;
    console_panel.height = UI_CONSOLE_HEIGHT;

    console_panel.es->container.left_padding = UI_LEFT_COLUMN_WIDTH + UI_TEXT_PADDING;
    console_panel.es->container.right_padding = UI_RIGHT_COLUMN_WIDTH + UI_TEXT_PADDING;
    console_panel.es->container.top_padding = win_state.win_height - (UI_FOOTER_HEIGHT + UI_CONSOLE_HEIGHT);
    console_panel.es->container.bottom_padding = UI_FOOTER_HEIGHT + UI_TEXT_PADDING;

    console_panel.es->render = true;
  }
  else
  {
    main_text_panel.x = UI_LEFT_COLUMN_WIDTH;
    main_text_panel.y = UI_FOOTER_HEIGHT;
    main_text_panel.width = win_state.win_width - UI_RIGHT_COLUMN_WIDTH;
    main_text_panel.height = win_state.win_height - (UI_TOP_HEADER_HEIGHT + UI_TOP_MENU_HEIGHT + UI_FILE_SWITCH_AREA_HEIGHT);

    main_text_panel.es->container.left_padding = UI_LEFT_COLUMN_WIDTH + UI_TEXT_PADDING;
    main_text_panel.es->container.right_padding = UI_RIGHT_COLUMN_WIDTH + UI_TEXT_PADDING;
    main_text_panel.es->container.top_padding = UI_TOP_HEADER_HEIGHT + UI_TOP_MENU_HEIGHT + UI_FILE_SWITCH_AREA_HEIGHT + UI_TEXT_PADDING;
    main_text_panel.es->container.bottom_padding = UI_FOOTER_HEIGHT + UI_TEXT_PADDING;

    console_panel.es->render = false;
  }

  main_text_panel.es->render = true;

  update_container(main_text_panel.es);
  update_container(console_panel.es);
}

void render_panels()
{
  if (main_text_panel.visible) render_interface_panel(&main_text_panel);
  if (console_panel.visible) render_interface_panel(&console_panel);
}

void update_console()
{
  s64 buffer_offset = 0;
	Editor_State* console_state = &console_es;

  // @TEMPORARY
  if (focused_editor_state == &console_es)
    return;

  copy_string(console_state->buffer + buffer_offset, "HoEXditor Console", sizeof "HoEXditor Console" - 1);
  buffer_offset = sizeof "HoEXditor Console" - 1;

	copy_string(console_state->buffer + buffer_offset, "\n\nCursor offset: ", sizeof "\n\nCursor offset: " - 1);
	buffer_offset += sizeof "\n\nCursor offset: " - 1;
	int n = s64_to_str_base10(main_text_es.cursor_info.cursor_offset, console_state->buffer + buffer_offset);
	buffer_offset += n;

	copy_string(console_state->buffer + buffer_offset, "\nNext line count: ", sizeof("\nNext line count: ") - 1);
	buffer_offset += sizeof("\nNext line count: ") - 1;
	n = s64_to_str_base10(main_text_es.cursor_info.next_line_count, console_state->buffer + buffer_offset);
	buffer_offset += n;

	copy_string(console_state->buffer + buffer_offset, "\nPrev line count: ", sizeof("\nPrev line count: ") - 1);
	buffer_offset += sizeof("\nPrev line count: ") - 1;
	n = s64_to_str_base10(main_text_es.cursor_info.previous_line_count, console_state->buffer + buffer_offset);
	buffer_offset += n;

	copy_string(console_state->buffer + buffer_offset, "\nSnap cursor column: ", sizeof("\nSnap cursor column: ") - 1);
	buffer_offset += sizeof("\nSnap cursor column: ") - 1;
	n = s64_to_str_base10(main_text_es.cursor_info.cursor_snaped_column, console_state->buffer + buffer_offset);
	buffer_offset += n;

	copy_string(console_state->buffer + buffer_offset, "\nCursor column: ", sizeof("\nCursor column: ") - 1);
	buffer_offset += sizeof("\nCursor column: ") - 1;
	n = s64_to_str_base10(main_text_es.cursor_info.cursor_column, console_state->buffer + buffer_offset);
	buffer_offset += n;

	copy_string(console_state->buffer + buffer_offset, "\nCursor line: ", sizeof("\nCursor line: ") - 1);
	buffer_offset += sizeof("\nCursor line: ") - 1;
	cursor_info cinfo = get_cursor_info(main_text_es.main_buffer_tid, main_text_es.cursor_info.cursor_offset);
	n = s64_to_str_base10(cinfo.line_number.lf, console_state->buffer + buffer_offset); //@error ? this should return not 0 when cursor is in the last line and the only char is \n
	buffer_offset += n;

  copy_string(console_state->buffer + buffer_offset, "\nText Size: ", sizeof("\nText Size: ") - 1);
	buffer_offset += sizeof("\nText Size: ") - 1;
	n = s64_to_str_base10(get_tid_text_size(main_text_es.main_buffer_tid), console_state->buffer + buffer_offset);
	buffer_offset += n;

  copy_string(console_state->buffer + buffer_offset, "\nBuffer Valid Bytes: ", sizeof("\nBuffer Valid Bytes: ") - 1);
	buffer_offset += sizeof("\nBuffer Valid Bytes: ") - 1;
	n = s64_to_str_base10(get_tid_valid_bytes(main_text_es.main_buffer_tid), console_state->buffer + buffer_offset);
	buffer_offset += n;

  copy_string(console_state->buffer + buffer_offset, "\nLast Line: ", sizeof("\nLast Line: ") - 1);
	buffer_offset += sizeof("\nLast Line: ") - 1;
	n = s64_to_str_base10(main_text_es.cursor_info.last_line, console_state->buffer + buffer_offset);
	buffer_offset += n;

	console_state->buffer_valid_bytes = buffer_offset;
}

void render_interface()
{
  Font_Rendering* previous_font = font_rendering;
  bind_font(&fd);
  update_font((float)win_state.win_width, (float)win_state.win_height);
  //render_top_header();
  render_file_switch_area();
  //render_text_area();
  render_left_column();
  render_right_column();
  render_footer();

  bind_font(&previous_font);

  update_console();
  update_panels_bounds();
  render_panels();

  if (is_interface_initialized) render_top_menu();
/*
  u8 word_to_search[256] = "Buddha";
	ho_search_result* result = search_word(focused_editor_state->main_buffer_id, 0, _tm_text_size[focused_editor_state->main_buffer_id] - 1, word_to_search, hstrlen(word_to_search));

	print("SEARCH RESULTS:\n");
	u32 num_results = 0;
	while (result != null)
	{
		print("%d. %d\n", ++num_results, result->cursor_position);
		void* last = result;
		result = result->next;
		hfree(last);
	}*/
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
  const float top_menu_item_initial_width_spacement = 5.0f;
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
  text_id tid = {.id = 0, .is_block_text = true}; // @TEMPORARY : should be dynamic
  u8* filename = get_tid_file_name(tid);  // @TEMPORARY : should be dynamic
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
    filename,
    hstrlen(filename),
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
    filename,
    hstrlen(filename),
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
