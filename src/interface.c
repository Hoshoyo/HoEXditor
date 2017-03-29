#include "interface.h"
#include "interface_definitions.h"
#include "os_dependent.h"
#include "font_rendering.h"
#include "text_manager.h"
#include "memory.h"
#include "input.h"
#include "console.h"
#include "dialog.h"
#include "text_events.h"

extern Window_State win_state;
extern u8* _tm_block_file_name;

Font_Rendering* fd;
interface_top_menu_item* _if_top_menu_items = null;
bool is_interface_initialized = false;

Editor_State* focused_editor_state = null;
interface_panel* _main_text_panels;
interface_panel* last_selected_main_text_panel;

// CONSOLE RELATED
interface_panel console_view_panel;
interface_panel console_input_panel;

// SEARCH RELATED
interface_panel search_view_panel;
interface_panel search_input_panel;

interface_panel* _auxiliar_panels;

// DIALOGS
ui_dialog* active_dialog;
ui_dialog* open_file_dialog;
ui_dialog* save_file_dialog;

// CONSTS
u8 default_file_name[] = UI_DEFAULT_FILE_NAME;

#define MOD(n) (n) > 0 ? (n) : -(n)

GLuint ui_icon_texture_id;

void init_interface()
{
	s32 width, height, channels;
	is_interface_initialized = true;

	fd = halloc(sizeof(Font_Rendering));
	load_font("c:/windows/fonts/consola.ttf", 16, &fd);
	fill_font(fd, win_state.win_width, win_state.win_height);
	bind_font(&fd);

	init_font(win_state.win_width, win_state.win_height);  // must be called only one time.

	prepare_editor_text(0, BATCH_SIZE);  // should be replaced
	prepare_editor_text(1, 1024);        // should be replaced

	init_console_window();
	init_search_window();

	u8* data = create_texture(UI_ICON_PATH, &width, &height, &channels);
	ui_icon_texture_id = gen_gl_texture(data, width, height);
	free_texture(data);

	prerender_top_menu();

	init_open_file_dialog();
	init_save_file_dialog();

	active_dialog = null;
}

void destroy_interface()
{
	is_interface_initialized = false;
	destroy_top_menu_prerender();
	release_font(&fd);
	destroy_dialog(open_file_dialog);
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

	update_active_dialog();	// this is an interface.c CALL, updates the active dialog.
	update_dialogs();		// this is a dialog.c CALL, updates all dialogs.
	render_dialogs();
}

Editor_State* ui_get_focused_editor()
{
	return focused_editor_state;
}

void ui_handle_mouse_click(s32 x, s32 y)
{
	if (focused_editor_state != null) cursor_change_by_click(focused_editor_state, x, y);
	handle_top_menu_click(null, x, y);
}

void ui_handle_key_down(s32 key)
{
	switch (key)
	{
		case VK_F1: {
			ui_change_console_window_visibility(!ui_is_console_window_visible());
		} break;
		case VK_F2: {
			if (_main_text_panels != null)
			{
				if (focused_editor_state != null && focused_editor_state->parent_panel->is_main_text_panel)
				{
					// If the current focused editor state belongs to a main_text_panel,
					// find the next main_text_panel of the same level and give it focus.
					interface_panel* new_main_text_panel = get_next_main_text_panel_by_level(focused_editor_state->parent_panel);
					change_main_text_panel_on_screen(new_main_text_panel, new_main_text_panel->main_text_panel_vertical_level);
					change_focused_editor(new_main_text_panel->es);
				}
				else
					// If the current focused editor state does not belong to a main_text_panel,
					// forces the focus to go to the main_text_panel of vertical level 0 (hardcoded) that is appearing on screen.
					change_focused_editor(get_on_screen_main_text_panel(0)->es);
			}
			else
				change_focused_editor(null); // If there are no main_text_panels, focused editor is null.
		} break;
		case VK_F3: {
			// Focused Editor State -> CONSOLE
			change_focused_editor(console_input_panel.es);
		} break;
		case VK_F4: {
			if (focused_editor_state != null && focused_editor_state->parent_panel->is_main_text_panel)
				close_panel(focused_editor_state->parent_panel);
		} break;
		case VK_F5: {
			if (active_dialog != null)
				change_focused_editor(active_dialog->input_panel->es);
		} break;
		case VK_F6: {
			if (focused_editor_state != null)
			{
				focused_editor_state->mode = next_mode(focused_editor_state);
				focused_editor_state->cursor_info.cursor_snaped_column = 0;
			}
		} break;
		case VK_F7: {
			if (focused_editor_state != null && focused_editor_state->parent_panel->is_main_text_panel)
			{
				s32 num_vertical_levels = get_number_of_main_text_vertical_levels();
				if (num_vertical_levels > 1)
				{
					interface_panel* current_focused_panel = focused_editor_state->parent_panel;
					s32 focused_panel_vertical_level = current_focused_panel->main_text_panel_vertical_level;
					interface_panel* new_focused_panel;

					if (focused_panel_vertical_level < num_vertical_levels - 1)
						new_focused_panel = get_on_screen_main_text_panel(focused_panel_vertical_level + 1);
					else
						new_focused_panel = get_on_screen_main_text_panel(0);

					change_focused_editor(new_focused_panel->es);
				}
			}
		} break;
		case VK_F8: {
			change_focused_editor(search_input_panel.es);
		} break;
		case VK_F9: {
			ui_change_search_window_visibility(!ui_is_search_window_visible());
		} break;
		case VK_F10: {
			recompile_font_shader();
		} break;
		case VK_ESCAPE: {
			if (active_dialog != null)
				close_dialog(active_dialog);
		}
		default: {

		} break;
	}
}

s32 ui_save_file(u8* file_path)
{
	if (focused_editor_state != null && focused_editor_state->parent_panel->is_main_text_panel)
	{
		if (file_path != null)
			return save_file(focused_editor_state->main_buffer_tid, file_path);
		else
		{
			// if no file_path was sent, it will try to save in file's original path
			u8* path_to_save = get_tid_file_name(focused_editor_state->main_buffer_tid);

			if (path_to_save != null)
				return save_file(focused_editor_state->main_buffer_tid, path_to_save);
			else
			{
				// if file has no original path, it will open save file dialog.
				ui_show_save_file_dialog();
				return 0;
			}
		}
	}

	return -1;
}

s32 ui_close_file()
{
	if (focused_editor_state != null && focused_editor_state->parent_panel->is_main_text_panel)
		return close_file(focused_editor_state->main_buffer_tid);

	return 0;
}

s32 ui_close_all_files()
{
	interface_panel* current_ip = _main_text_panels;

	while (current_ip != null)
	{
		interface_panel* ip_to_be_deleted = current_ip;
		current_ip = current_ip->next;
		close_file(ip_to_be_deleted->es->main_buffer_tid);
	}

	return 0;
}

s32 ui_search_word(u8* word, s64 word_length)
{
	interface_panel* screen_panel = last_selected_main_text_panel;

	if (screen_panel == null)
		return -1;

	Editor_State* es = screen_panel->es;
	text_id tid = es->main_buffer_tid;

	// If cursor_position is higher than the end of the text, it must be forced to be the same as the end of the text.
	s64 cursor_position = (es->cursor_info.cursor_offset < get_tid_text_size(tid)) ?
		es->cursor_info.cursor_offset : get_tid_text_size(tid) - 1;

	s64 cursor_result;
	
	if (es->selecting)
	{
		s64 bytes_selected = MOD(es->cursor_info.selection_offset - cursor_position);
		s64 cursor_begin = MIN(es->cursor_info.selection_offset, cursor_position);

		cursor_result = find_next_pattern_forward(tid, cursor_begin, cursor_begin + bytes_selected, word, word_length);
	}
	else
	{
		u64 last_cursor = get_tid_text_size(tid) - 1;
		cursor_result = find_next_pattern_forward(tid, cursor_position, last_cursor, word, word_length);
		if (cursor_result < 0)
		{
			last_cursor = MIN(cursor_position + word_length, last_cursor);
			cursor_result = find_next_pattern_forward(tid, 0, last_cursor, word, word_length);
		}
	}


	if (cursor_result < 0)
		return -2;

	editor_force_selection(es, cursor_result, cursor_result + word_length);
	change_focused_editor(es);

	return 0;
}

s32 ui_clone_and_split_view()
{
	if (focused_editor_state != null && focused_editor_state->parent_panel->is_main_text_panel)
	{
		interface_panel* panel = focused_editor_state->parent_panel;
		interface_panel* new_panel = insert_clone_main_text_window(panel->es->main_buffer_tid);

		if (panel->main_text_panel_vertical_level == 0)
			change_main_text_panel_on_screen(new_panel, 1);
		else
			change_main_text_panel_on_screen(new_panel, 0);
	}

	return 0;
}

s32 ui_split_view()
{
	if (focused_editor_state != null && focused_editor_state->parent_panel->is_main_text_panel)
	{
		interface_panel* panel = focused_editor_state->parent_panel;

		if (panel->main_text_panel_vertical_level == 0)
		{
			interface_panel* replacement_panel = get_next_main_text_panel_by_level(panel);
			if (replacement_panel == panel)
				return -1;

			change_main_text_panel_on_screen(panel, 1);
			change_main_text_panel_on_screen(replacement_panel, 0);
		}
		else
		{
			interface_panel* replacement_panel = get_next_main_text_panel_by_level(panel);
			if (replacement_panel == panel)
				return -1;

			change_main_text_panel_on_screen(panel, 0);
			change_main_text_panel_on_screen(replacement_panel, 1);
		}
	}

	return 0;
}

s32 close_file(text_id tid)
{
	interface_panel* related_panel = _main_text_panels;

	while (related_panel != null)
	{
		if (related_panel->es->main_buffer_tid.id == tid.id) break;
		related_panel = related_panel->next;
	}

	if (related_panel == null) return -1;

	return close_panel(related_panel);
}

s32 close_panel(interface_panel* panel)
{
	s32 total_number_of_vertical_level = get_number_of_main_text_vertical_levels();
	s32 panel_vertical_level = panel->main_text_panel_vertical_level;

	if (last_selected_main_text_panel == panel)
		last_selected_main_text_panel = null;

	interface_panel* new_panel_on_screen = remove_main_text_window(panel);

	if (new_panel_on_screen != null)
	{
		// If new_panel_screen != null, then there were more than one panel in that vertical level
		// which means that we can just give screen to the new_panel_on_screen returned and that's all.
		change_main_text_panel_on_screen(new_panel_on_screen, new_panel_on_screen->main_text_panel_vertical_level);
		change_focused_editor(new_panel_on_screen->es);
	}
	else
	{
		// If new_panel_screen == null, then the deleted panel was the only panel that had that vertical level
		// which means that its level will not exist anymore. However, there is a chance that higher levels exist
		// (for example: the deleted panel had level 1, but level 0, 1 and 2 exists). In that case, all higher levels
		// must be decremented, so the code will be consistent. [Levels must start at 0 and must not have holes]

		// Decrement all levels
		s32 current_level;
		interface_panel* current_panel;
		interface_panel* aux;

		for (current_level = panel_vertical_level + 1; current_level < total_number_of_vertical_level; ++current_level)
		{
			current_panel = get_first_main_text_panel(current_level);
			aux = current_panel;

			while (aux != null)
			{
				--aux->main_text_panel_vertical_level;
				aux = get_first_main_text_panel(current_level);
			}
		}

		// Change focus to first panel
		if (_main_text_panels != null)
			change_focused_editor(get_first_main_text_panel(0)->es);
		else
			change_focused_editor(null);
	}

	return 0;
}

s32 ui_open_file(bool empty, u8* file_path)
{
	interface_panel* new_panel_on_screen = insert_main_text_window(empty, file_path);
	change_main_text_panel_on_screen(new_panel_on_screen, new_panel_on_screen->main_text_panel_vertical_level);
	change_focused_editor(new_panel_on_screen->es);
	return 0;
}

void ui_handle_file_drop(u8* path, s32 x, s32 y)
{
	ui_open_file(false, path);
}

void ui_show_open_file_dialog()
{
	open_dialog(open_file_dialog);
}

void ui_show_save_file_dialog()
{
	open_dialog(save_file_dialog);
}

void open_dialog(ui_dialog* dialog)
{
	change_dialog_visibility(dialog, true);
	active_dialog = dialog;
	change_focused_editor(active_dialog->input_panel->es);
}

void close_dialog(ui_dialog* dialog)
{
	change_dialog_visibility(dialog, false);
	active_dialog = null;
	if (focused_editor_state == dialog->input_panel->es)
	{
		interface_panel* panel_to_get_focus = last_selected_main_text_panel;
		if (panel_to_get_focus != null)
			change_focused_editor(panel_to_get_focus->es);
		else
			change_focused_editor(null);
	}
}

void update_active_dialog()
{
	if (active_dialog != null)
	{
		active_dialog->x = round((win_state.win_width - active_dialog->width) / 2.0f);
		active_dialog->y = round((win_state.win_height - active_dialog->height) / 2.0f);
	}
}

void init_open_file_dialog()
{
	u8 open_file_dialog_text[] = "Open File\n\nPath:";
	u32 open_file_dialog_text_size = sizeof("Open File\n\nPath:") - 1;
	open_file_dialog = create_dialog(100.0f,
		100.0f, 100.0f, 300.0f, 0.5f,
		UI_DIALOG_VIEW_FONT_COLOR,
		UI_DIALOG_VIEW_CURSOR_COLOR,
		UI_DIALOG_VIEW_BACKGROUND_COLOR,
		UI_DIALOG_INPUT_FONT_COLOR,
		UI_DIALOG_INPUT_CURSOR_COLOR,
		UI_DIALOG_INPUT_BACKGROUND_COLOR,
		true, open_file_dialog_callback);
	change_view_content(open_file_dialog, open_file_dialog_text, open_file_dialog_text_size);
}

void init_save_file_dialog()
{
	u8 save_file_dialog_text[] = "Save File\n\nPath:";
	u32 save_file_dialog_text_size = sizeof("Save File\n\nPath:") - 1;
	save_file_dialog = create_dialog(100.0f,
		100.0f, 100.0f, 300.0f, 0.5f,
		UI_DIALOG_VIEW_FONT_COLOR,
		UI_DIALOG_VIEW_CURSOR_COLOR,
		UI_DIALOG_VIEW_BACKGROUND_COLOR,
		UI_DIALOG_INPUT_FONT_COLOR,
		UI_DIALOG_INPUT_CURSOR_COLOR,
		UI_DIALOG_INPUT_BACKGROUND_COLOR,
		true, save_file_dialog_callback);
	change_view_content(save_file_dialog, save_file_dialog_text, save_file_dialog_text_size);
}

void open_file_dialog_callback(u8* text)
{
	ui_open_file(false, text);
	close_dialog(open_file_dialog);
}

void save_file_dialog_callback(u8* text)
{
	ui_save_file(text);
	close_dialog(save_file_dialog);
}

float get_main_panel_width_per_level(s32 vertical_level)
{
	s32 vertical_levels_num = get_number_of_main_text_vertical_levels();

	// @TODO: CHECK FLOOR
	return floor((win_state.win_width - UI_LEFT_COLUMN_WIDTH - UI_RIGHT_COLUMN_WIDTH) / ((float)vertical_levels_num));
}

// @TEMPORARY: OPTIMIZE
s32 get_number_of_main_text_vertical_levels()
{
	s32 v_levels_num = 0;

	while (get_first_main_text_panel(v_levels_num) != null)
		++v_levels_num;

	return v_levels_num;
}

interface_panel* get_first_main_text_panel(s32 vertical_level)
{
	if (_main_text_panels == null)
		return null;

	interface_panel* current_interface_panel = _main_text_panels;

	do
	{
		if (current_interface_panel->main_text_panel_vertical_level == vertical_level)
			return current_interface_panel;
		current_interface_panel = current_interface_panel->next;
	} while (current_interface_panel != null);

	return null;
}

interface_panel* get_on_screen_main_text_panel(s32 vertical_level)
{
	if (_main_text_panels == null)
		return null;

	interface_panel* current_interface_panel = _main_text_panels;

	do
	{
		if (current_interface_panel->main_text_panel_vertical_level == vertical_level)
			if (current_interface_panel->visible)
				return current_interface_panel;
		current_interface_panel = current_interface_panel->next;
	} while (current_interface_panel != null);

	return null;
}

interface_panel* get_next_main_text_panel_by_level(interface_panel* reference_panel)
{
	if (reference_panel == null || _main_text_panels == null)
		return null;

	interface_panel* current_interface_panel = reference_panel->next;

	// Search from current_interface_panel to the end
	while (current_interface_panel != null)
	{
		if (current_interface_panel->main_text_panel_vertical_level == reference_panel->main_text_panel_vertical_level)
			return current_interface_panel;
		current_interface_panel = current_interface_panel->next;
	}

	// Search from the beggining to current_interface_panel
	current_interface_panel = _main_text_panels;

	while (current_interface_panel != reference_panel)
	{
		if (current_interface_panel->main_text_panel_vertical_level == reference_panel->main_text_panel_vertical_level)
			return current_interface_panel;
		current_interface_panel = current_interface_panel->next;
	}

	return current_interface_panel;
}

interface_panel* insert_main_text_window(bool empty, u8* filename)
{
	Editor_State* main_text_es = halloc(sizeof(Editor_State));
	interface_panel* main_text_panel = halloc(sizeof(interface_panel));

	create_tid(&main_text_es->main_buffer_tid, true);

	// init main_text_es
	init_editor_state(main_text_es);
	main_text_es->font_color = UI_MAIN_TEXT_COLOR;
	main_text_es->cursor_color = UI_MAIN_TEXT_CURSOR_COLOR;
	main_text_es->line_number_color = UI_MAIN_TEXT_LINE_NUMBER_COLOR;
	main_text_es->show_cursor = false;
	main_text_es->render_line_numbers = true;
	main_text_es->parent_panel = main_text_panel;

	if (!empty) load_file(main_text_es->main_buffer_tid, filename);
	setup_view_buffer(main_text_es, 0, SCREEN_BUFFER_SIZE, true);

	main_text_panel->es = main_text_es;
	main_text_panel->x = main_text_es->container.left_padding;
	main_text_panel->y = main_text_es->container.bottom_padding;
	main_text_panel->width = main_text_es->container.right_padding - main_text_es->container.left_padding;
	main_text_panel->height = main_text_es->container.top_padding - main_text_es->container.bottom_padding;
	main_text_panel->background_color = UI_TEXT_AREA_COLOR;
	main_text_panel->visible = false;
	main_text_panel->is_main_text_panel = true;
	main_text_panel->main_text_panel_vertical_level = 0;

	// Add new interface_panel_list to global list
	if (_main_text_panels == null)
	{
		main_text_panel->next = null;
		main_text_panel->previous = null;
		_main_text_panels = main_text_panel;
	}
	else
	{
		interface_panel* last_panel = _main_text_panels;

		// Find last panel
		while (last_panel->next != null)
			last_panel = last_panel->next;

		last_panel->next = main_text_panel;
		main_text_panel->next = null;
		main_text_panel->previous = last_panel;
	}

	return main_text_panel;
}

interface_panel* insert_clone_main_text_window(text_id tid)
{
	Editor_State* main_text_es = halloc(sizeof(Editor_State));
	interface_panel* main_text_panel = halloc(sizeof(interface_panel));

	add_tid_reference(tid);
	main_text_es->main_buffer_tid = tid;

	// init main_text_es
	init_editor_state(main_text_es);
	main_text_es->font_color = UI_MAIN_TEXT_COLOR;
	main_text_es->cursor_color = UI_MAIN_TEXT_CURSOR_COLOR;
	main_text_es->line_number_color = UI_MAIN_TEXT_LINE_NUMBER_COLOR;
	main_text_es->show_cursor = false;
	main_text_es->render_line_numbers = true;
	main_text_es->parent_panel = main_text_panel;

	setup_view_buffer(main_text_es, 0, SCREEN_BUFFER_SIZE, true);

	main_text_panel->es = main_text_es;
	main_text_panel->x = main_text_es->container.left_padding;
	main_text_panel->y = main_text_es->container.bottom_padding;
	main_text_panel->width = main_text_es->container.right_padding - main_text_es->container.left_padding;
	main_text_panel->height = main_text_es->container.top_padding - main_text_es->container.bottom_padding;
	main_text_panel->background_color = UI_TEXT_AREA_COLOR;
	main_text_panel->visible = false;
	main_text_panel->is_main_text_panel = true;
	main_text_panel->main_text_panel_vertical_level = 0;

	// Add new interface_panel_list to global list
	if (_main_text_panels == null)
	{
		main_text_panel->next = null;
		main_text_panel->previous = null;
		_main_text_panels = main_text_panel;
	}
	else
	{
		interface_panel* last_panel = _main_text_panels;

		// Find last panel
		while (last_panel->next != null)
			last_panel = last_panel->next;

		last_panel->next = main_text_panel;
		main_text_panel->next = null;
		main_text_panel->previous = last_panel;
	}

	return main_text_panel;
}

interface_panel* remove_main_text_window(interface_panel* main_text_panel)
{
	s32 aux;
	interface_panel* next_panel_that_should_assume_screen;
	interface_panel* main_text_panel_on_screen_of_same_level =
		get_on_screen_main_text_panel(main_text_panel->main_text_panel_vertical_level);

	// Find out which will be the next main_text_panel of the same level that will assume the screen.
	if (main_text_panel != main_text_panel_on_screen_of_same_level)
		next_panel_that_should_assume_screen = main_text_panel_on_screen_of_same_level;
	else
	{
		next_panel_that_should_assume_screen = get_next_main_text_panel_by_level(main_text_panel);

		// If next_panel_that_should_assume_screen is equal to main_text_panel, it must be forced to become null,
		// since main_text_panel will be deleted.
		if (next_panel_that_should_assume_screen == main_text_panel)
			next_panel_that_should_assume_screen = null;
	}

	if (main_text_panel->previous != null)
		main_text_panel->previous->next = main_text_panel->next;
	if (main_text_panel->next != null)
		main_text_panel->next->previous = main_text_panel->previous;

	if (_main_text_panels == main_text_panel)
		_main_text_panels = main_text_panel->next;

	free_interface_panel(main_text_panel);

	return next_panel_that_should_assume_screen;
}

void free_interface_panel(interface_panel* interface_panel)
{
	finalize_tid(interface_panel->es->main_buffer_tid);
	hfree(interface_panel->es);
	hfree(interface_panel);
}

void init_console_window()
{
	/* CONSOLE VIEW */
	Editor_State* console_view_es = halloc(sizeof(Editor_State));
	create_tid(&console_view_es->main_buffer_tid, false);

	init_editor_state(console_view_es);
	console_view_es->font_color = UI_CONSOLE_VIEW_TEXT_COLOR;
	console_view_es->cursor_color = UI_CONSOLE_VIEW_CURSOR_COLOR;
	console_view_es->line_number_color = (vec4) { 0, 0, 0, 0 };
	console_view_es->parent_panel = &console_view_panel;

	create_real_buffer(console_view_es->main_buffer_tid, UI_CONSOLE_VIEW_BUFFER_SIZE);
	setup_view_buffer(console_view_es, 0, UI_CONSOLE_VIEW_BUFFER_SIZE, true);

	console_view_panel.es = console_view_es;
	console_view_panel.height = UI_CONSOLE_VIEW_HEIGHT;
	console_view_panel.background_color = UI_CONSOLE_VIEW_BACKGROUND_COLOR;
	console_view_panel.visible = true;
	console_view_panel.is_main_text_panel = false;
	console_view_panel.main_text_panel_vertical_level = 0;

	/* CONSOLE INPUT */
	Editor_State* console_input_es = halloc(sizeof(Editor_State));
	create_tid(&console_input_es->main_buffer_tid, false);

	init_editor_state(console_input_es);
	console_input_es->font_color = UI_CONSOLE_INPUT_TEXT_COLOR;
	console_input_es->cursor_color = UI_CONSOLE_INPUT_CURSOR_COLOR;
	console_input_es->line_number_color = (vec4) { 0, 0, 0, 0 };
	console_input_es->parent_panel = &console_input_panel;

	console_input_es->individual_char_handler = console_char_handler;

	create_real_buffer(console_input_es->main_buffer_tid, UI_CONSOLE_INPUT_BUFFER_SIZE);
	setup_view_buffer(console_input_es, 0, UI_CONSOLE_INPUT_BUFFER_SIZE, true);

	console_input_panel.es = console_input_es;
	console_input_panel.height = UI_CONSOLE_INPUT_HEIGHT;
	console_input_panel.background_color = UI_CONSOLE_INPUT_BACKGROUND_COLOR;
	console_input_panel.visible = true;
	console_input_panel.is_main_text_panel = false;
	console_input_panel.main_text_panel_vertical_level = 0;
	
	add_auxiliar_panel(&console_input_panel);
	add_auxiliar_panel(&console_view_panel);
}

void init_search_window()
{
	/* SEARCH VIEW */
	Editor_State* search_view_es = halloc(sizeof(Editor_State));
	create_tid(&search_view_es->main_buffer_tid, false);

	init_editor_state(search_view_es);
	search_view_es->font_color = UI_SEARCH_VIEW_TEXT_COLOR;
	search_view_es->cursor_color = UI_SEARCH_VIEW_CURSOR_COLOR;
	search_view_es->line_number_color = (vec4) { 0, 0, 0, 0 };
	search_view_es->parent_panel = &search_view_panel;

	create_real_buffer(search_view_es->main_buffer_tid, UI_SEARCH_VIEW_BUFFER_SIZE);
	setup_view_buffer(search_view_es, 0, UI_SEARCH_VIEW_BUFFER_SIZE, true);

	search_view_panel.es = search_view_es;
	search_view_panel.height = UI_SEARCH_VIEW_HEIGHT;
	search_view_panel.background_color = UI_SEARCH_VIEW_BACKGROUND_COLOR;
	search_view_panel.visible = true;
	search_view_panel.is_main_text_panel = false;
	search_view_panel.main_text_panel_vertical_level = 0;

	/* SEARCH INPUT */
	Editor_State* search_input_es = halloc(sizeof(Editor_State));
	create_tid(&search_input_es->main_buffer_tid, false);

	init_editor_state(search_input_es);
	search_input_es->font_color = UI_SEARCH_INPUT_TEXT_COLOR;
	search_input_es->cursor_color = UI_SEARCH_INPUT_CURSOR_COLOR;
	search_input_es->line_number_color = (vec4) { 0, 0, 0, 0 };
	search_input_es->parent_panel = &search_input_panel;

	create_real_buffer(search_input_es->main_buffer_tid, UI_SEARCH_INPUT_BUFFER_SIZE);
	setup_view_buffer(search_input_es, 0, UI_SEARCH_INPUT_BUFFER_SIZE, true);

	search_input_panel.es = search_input_es;
	search_input_panel.height = UI_SEARCH_INPUT_HEIGHT;
	search_input_panel.background_color = UI_SEARCH_INPUT_BACKGROUND_COLOR;
	search_input_panel.visible = true;
	search_input_panel.is_main_text_panel = false;
	search_input_panel.main_text_panel_vertical_level = 0;

	search_input_es->individual_char_handler = search_char_handler;

	// @temporary : instructions
	u8 search_view_text[] = "Search Word: [F8] Focus [F9] Hide";
	s32 search_view_text_size = sizeof("Search Word: [F8] Focus [F9] Hide") - 1;
	insert_text(search_view_es->main_buffer_tid, search_view_text, search_view_text_size, 0);
	// -------------------------

	add_auxiliar_panel(&search_input_panel);
	add_auxiliar_panel(&search_view_panel);
}

bool ui_is_console_window_visible()
{
	// this function is ignoring console_view_panel visibility
	return console_input_panel.visible;
}

bool ui_is_search_window_visible()
{
	// this function is ignoring search_view_panel visibility
	return search_input_panel.visible;
}

void ui_change_console_window_visibility(bool visible)
{
	console_view_panel.visible = !console_view_panel.visible;
	console_input_panel.visible = !console_input_panel.visible;
}

void ui_change_search_window_visibility(bool visible)
{
	search_view_panel.visible = !search_view_panel.visible;
	search_input_panel.visible = !search_input_panel.visible;
}

s32 search_char_handler(Editor_State* es, s32 key)
{
	static s64 last_cursor_position = 0;

	if (key == CARRIAGE_RETURN_KEY)
	{
		s32 word_size = get_tid_text_size(es->main_buffer_tid) * sizeof(u8);
		u8* word = halloc(word_size * sizeof(u8));
		delete_text(es->main_buffer_tid, word, word_size, 0);
		cursor_force(es, 0);

		ui_search_word(word, word_size);

		return 0;
	}

	return -1;
}

void add_auxiliar_panel(interface_panel* auxiliar_panel)
{
	if (_auxiliar_panels == null)
	{
		auxiliar_panel->next = null;
		auxiliar_panel->previous = null;
		_auxiliar_panels = auxiliar_panel;
	}
	else
	{
		interface_panel* current_panel = _auxiliar_panels;

		while (current_panel->next != null)
			current_panel = current_panel->next;

		current_panel->next = auxiliar_panel;
		auxiliar_panel->previous = current_panel;
		auxiliar_panel->next = null;
	}
}

void render_interface_panel(interface_panel* panel)
{
	if (panel->visible)
	{
		/*vec4 c = UI_RED_COLOR;
		render_transparent_quad_with_border(panel->x, panel->y, panel->x + panel->width, panel->y + panel->height, &panel->background_color, &UI_RED_COLOR,
		0,  // top
		0,  // bottom
		0,  // left
		0); // right */

		render_transparent_quad(panel->x, panel->y, panel->x + panel->width, panel->y + panel->height, &panel->background_color);
		update_and_render_editor(panel->es);
	}
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
	if (focused_editor_state != null)
		focused_editor_state->show_cursor = false;
	focused_editor_state = es;
	if (focused_editor_state != null)
		focused_editor_state->show_cursor = true;

	// Refresh last_selected_main_panel
	if (focused_editor_state != null && focused_editor_state->parent_panel->is_main_text_panel)
		last_selected_main_text_panel = focused_editor_state->parent_panel;
}

void change_main_text_panel_on_screen(interface_panel* panel, s32 vertical_level)
{
	interface_panel* current_interface_panel = _main_text_panels;

	do
	{
		if (current_interface_panel->main_text_panel_vertical_level == vertical_level)
			current_interface_panel->visible = false;
		current_interface_panel = current_interface_panel->next;
	} while (current_interface_panel != null);

	if (panel != null)
	{
		panel->visible = true;
		panel->main_text_panel_vertical_level = vertical_level;
	}
}

// It will update every panel attribute to fit screen
// The only attribute that is mantained and used is the 'height' of the auxiliar panels.
void update_panels_bounds()
{
	interface_panel* auxiliar_panel;
	s32 current_vertical_level;
	s32 vertical_levels_num = get_number_of_main_text_vertical_levels();

	/* WIDTH-RELATED ATTRIBUTES UPDATE */

	// Main Text Panel On Screen Width Update
	for (current_vertical_level = 0; current_vertical_level < vertical_levels_num; ++current_vertical_level)
	{
		float current_level_panel_width = get_main_panel_width_per_level(current_vertical_level);
		auxiliar_panel = get_on_screen_main_text_panel(current_vertical_level);
		auxiliar_panel->x = UI_LEFT_COLUMN_WIDTH + current_vertical_level * current_level_panel_width;
		auxiliar_panel->width = current_level_panel_width;
		auxiliar_panel->es->container.left_padding = auxiliar_panel->x + UI_TEXT_PADDING;
		auxiliar_panel->es->container.right_padding = win_state.win_width - auxiliar_panel->x - auxiliar_panel->width;
	}

	// Auxiliar Panels Width Update
	auxiliar_panel = _auxiliar_panels;

	while (auxiliar_panel != null)
	{
		if (auxiliar_panel->visible)
		{
			auxiliar_panel->x = UI_LEFT_COLUMN_WIDTH;
			auxiliar_panel->width = win_state.win_width - UI_RIGHT_COLUMN_WIDTH - auxiliar_panel->x;
			auxiliar_panel->es->container.left_padding = UI_LEFT_COLUMN_WIDTH + UI_TEXT_PADDING;
			auxiliar_panel->es->container.right_padding = UI_RIGHT_COLUMN_WIDTH + UI_TEXT_PADDING;
		}

		auxiliar_panel = auxiliar_panel->next;
	}

	/* HEIGHT-RELATED ATTRIBUTES UPDATE */

	// Calculation of total height occupied by auxiliar panels:
	u32 total_height = 0;
	auxiliar_panel = _auxiliar_panels;

	while (auxiliar_panel != null)
	{
		if (auxiliar_panel->visible)
			total_height += auxiliar_panel->height;
		auxiliar_panel = auxiliar_panel->next;
	}

	// Main Text Panel On Screen Height Update
	for (current_vertical_level = 0; current_vertical_level < vertical_levels_num; ++current_vertical_level)
	{
		auxiliar_panel = get_on_screen_main_text_panel(current_vertical_level);
		auxiliar_panel->y = UI_FOOTER_HEIGHT + total_height;
		auxiliar_panel->height = win_state.win_height - (UI_TOP_HEADER_HEIGHT + UI_TOP_MENU_HEIGHT + UI_FILE_SWITCH_AREA_HEIGHT) - auxiliar_panel->y;
		auxiliar_panel->es->container.top_padding = UI_TOP_HEADER_HEIGHT + UI_TOP_MENU_HEIGHT + UI_FILE_SWITCH_AREA_HEIGHT + UI_TEXT_PADDING;
		auxiliar_panel->es->container.bottom_padding = UI_FOOTER_HEIGHT + UI_TEXT_PADDING + total_height;
	}

	// Auxiliar Panels Width Update
	auxiliar_panel = _auxiliar_panels;
	total_height = 0;
	
	while (auxiliar_panel != null)
	{
		if (auxiliar_panel->visible)
		{
			auxiliar_panel->y = UI_FOOTER_HEIGHT + total_height;
			auxiliar_panel->es->container.top_padding = win_state.win_height - (UI_FOOTER_HEIGHT + total_height + auxiliar_panel->height);
			auxiliar_panel->es->container.bottom_padding = UI_FOOTER_HEIGHT + UI_TEXT_PADDING + total_height;

			// @TODO: reposition code below
			auxiliar_panel->es->render = true;

			total_height += auxiliar_panel->height;
		}

		auxiliar_panel = auxiliar_panel->next;
	}

	for (current_vertical_level = 0; current_vertical_level < vertical_levels_num; ++current_vertical_level)
	{
		auxiliar_panel = get_on_screen_main_text_panel(current_vertical_level);
		auxiliar_panel->es->render = true;
		update_container(auxiliar_panel->es);
	}

	// Update Containers
	auxiliar_panel = _auxiliar_panels;
	while (auxiliar_panel != null)
	{
		if (auxiliar_panel->visible)
			update_container(auxiliar_panel->es);
		auxiliar_panel = auxiliar_panel->next;
	}
}

void render_panels()
{
	interface_panel* main_text_panels = _main_text_panels;
	while (main_text_panels != null)
	{
		render_interface_panel(main_text_panels);
		main_text_panels = main_text_panels->next;
	}
	interface_panel* auxiliar_panels = _auxiliar_panels;
	while (auxiliar_panels != null)
	{
		render_interface_panel(auxiliar_panels);
		auxiliar_panels = auxiliar_panels->next;
	}
	render_interface_panel(&console_view_panel);
	render_interface_panel(&console_input_panel);
}

void render_top_header()
{
	const vec2 icon_x_position = (vec2) { 15.0f, 35.0f };
	const vec2 icon_y_position = (vec2) { 10.0f, 30.0f };
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
			if (top_menu_item->parent != null)
				top_menu_item->parent->is_sub_container_open = false;
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
	interface_size size = { .width = 0,.height = 0 };
	Font_RenderInInfo font_in = { 0 };
	Font_RenderOutInfo font_out;

	for (aux = 0; aux < list_size; aux++)
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
	  {.name = UI_MENU_ITEM_1,.type = T_UI_MENU_ITEM_1},
	  {.name = UI_MENU_ITEM_2,.type = T_UI_MENU_ITEM_2},
	  {.name = UI_MENU_ITEM_3,.type = T_UI_MENU_ITEM_3},
	  {.name = UI_MENU_ITEM_4,.type = T_UI_MENU_ITEM_4},
	};

	interface_top_menu_item_id sub_menu_items_1[] = {
	  {.name = UI_SUBMENU_ITEM_1_1,.type = T_UI_SUBMENU_ITEM_1_1},
	  {.name = UI_SUBMENU_ITEM_1_2,.type = T_UI_SUBMENU_ITEM_1_2},
	  {.name = UI_SUBMENU_ITEM_1_3,.type = T_UI_SUBMENU_ITEM_1_3},
	  {.name = UI_SUBMENU_ITEM_1_4,.type = T_UI_SUBMENU_ITEM_1_4},
	  {.name = UI_SUBMENU_ITEM_1_5,.type = T_UI_SUBMENU_ITEM_1_5},
	  {.name = UI_SUBMENU_ITEM_1_6,.type = T_UI_SUBMENU_ITEM_1_6},
	  {.name = UI_SUBMENU_ITEM_1_7,.type = T_UI_SUBMENU_ITEM_1_7},
	  {.name = UI_SUBMENU_ITEM_1_8,.type = T_UI_SUBMENU_ITEM_1_8},
	};

	interface_top_menu_item_id sub_menu_items_2[] = {
	  {.name = UI_SUBMENU_ITEM_2_1,.type = T_UI_SUBMENU_ITEM_2_1},
	  {.name = UI_SUBMENU_ITEM_2_2,.type = T_UI_SUBMENU_ITEM_2_2},
	  {.name = UI_SUBMENU_ITEM_2_3,.type = T_UI_SUBMENU_ITEM_2_3},
	  {.name = UI_SUBMENU_ITEM_2_4,.type = T_UI_SUBMENU_ITEM_2_4},
	  {.name = UI_SUBMENU_ITEM_2_5,.type = T_UI_SUBMENU_ITEM_2_5},
	  {.name = UI_SUBMENU_ITEM_2_6,.type = T_UI_SUBMENU_ITEM_2_6},
	  {.name = UI_SUBMENU_ITEM_2_7,.type = T_UI_SUBMENU_ITEM_2_7},
	  {.name = UI_SUBMENU_ITEM_2_8,.type = T_UI_SUBMENU_ITEM_2_8},
	  {.name = UI_SUBMENU_ITEM_2_9,.type = T_UI_SUBMENU_ITEM_2_9},
	};

	interface_top_menu_item_id sub_menu_items_3[] = {
	  {.name = UI_SUBMENU_ITEM_3_1,.type = T_UI_SUBMENU_ITEM_3_1},
	  {.name = UI_SUBMENU_ITEM_3_2,.type = T_UI_SUBMENU_ITEM_3_2},
	  {.name = UI_SUBMENU_ITEM_3_3,.type = T_UI_SUBMENU_ITEM_3_3},
	  {.name = UI_SUBMENU_ITEM_3_4,.type = T_UI_SUBMENU_ITEM_3_4},
	  {.name = UI_SUBMENU_ITEM_3_5,.type = T_UI_SUBMENU_ITEM_3_5},
	  {.name = UI_SUBMENU_ITEM_3_6,.type = T_UI_SUBMENU_ITEM_3_6},
	  {.name = UI_SUBMENU_ITEM_3_7,.type = T_UI_SUBMENU_ITEM_3_7},
	  {.name = UI_SUBMENU_ITEM_3_8,.type = T_UI_SUBMENU_ITEM_3_8},
	};

	interface_top_menu_item_id sub_menu_items_4[] = {
	  {.name = UI_SUBMENU_ITEM_4_1,.type = T_UI_SUBMENU_ITEM_4_1},
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
	float top_menu_item_height_spacement = round((top_menu_max_height - top_menu_min_height - fd->max_height) / 2.0f);
	vec4 top_menu_color = UI_BACKGROUND_COLOR;
	vec4 top_menu_text_color = UI_TOP_MENU_TEXT_COLOR;
	vec4 top_menu_selection_color = UI_TOP_MENU_SELECTION_COLOR;
	vec4 sub_menu_selection_color = UI_SUB_MENU_SELECTION_COLOR;
	s32 submenu_items_size;
	interface_size bounds;
	Font_RenderInInfo font_in_info = { 0 };
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
	submenu_items_size = sizeof(sub_menu_items_1) / sizeof(interface_top_menu_item_id);
	bounds = get_submenu_bounds(sub_menu_items_1, submenu_items_size, top_submenu_item_initial_height_spacement);

	for (u32 aux = 0; aux < submenu_items_size; ++aux)
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
	submenu_items_size = sizeof(sub_menu_items_2) / sizeof(interface_top_menu_item_id);
	bounds = get_submenu_bounds(sub_menu_items_2, submenu_items_size, top_submenu_item_initial_height_spacement);

	for (u32 aux = 0; aux < submenu_items_size; ++aux)
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
	submenu_items_size = sizeof(sub_menu_items_3) / sizeof(interface_top_menu_item_id);
	bounds = get_submenu_bounds(sub_menu_items_3, submenu_items_size, top_submenu_item_initial_height_spacement);

	for (u32 aux = 0; aux < submenu_items_size; ++aux)
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
	submenu_items_size = sizeof(sub_menu_items_4) / sizeof(interface_top_menu_item_id);
	bounds = get_submenu_bounds(sub_menu_items_4, submenu_items_size, top_submenu_item_initial_height_spacement);

	for (u32 aux = 0; aux < submenu_items_size; ++aux)
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
	top_menu_item->parent = null;

	interface_top_menu_item* next_item = items;

	// IMPORTANT: The field 'parent' inside interface_top_menu_item struct
	// is filled here. This is 'hidden'.
	while (next_item != null)
	{
		next_item->parent = top_menu_item;
		next_item = next_item->next;
	}

	if (*root != null)
	{
		interface_top_menu_item* aux = *root;

		while (aux->next != null)
			aux = aux->next;

		aux->next = top_menu_item;
	}
	else
		*root = top_menu_item;

	return top_menu_item;
}

void render_file_switch_area()
{
	interface_panel* auxiliar_panel;
	s32 current_vertical_level;
	float last_max_width = 0;
	const float file_name_width_spacement = 5.0f;
	const float file_name_height_spacement = 5.0f;
	const float spacement_per_files = 5.0f;
	float file_switch_area_min_height = win_state.win_height - UI_TOP_HEADER_HEIGHT - UI_TOP_MENU_HEIGHT - UI_FILE_SWITCH_AREA_HEIGHT;
	float file_switch_area_max_height = win_state.win_height - UI_TOP_HEADER_HEIGHT - UI_TOP_MENU_HEIGHT;
	float file_switch_area_min_width = UI_LEFT_COLUMN_WIDTH;
	float file_switch_area_max_width = win_state.win_width - UI_RIGHT_COLUMN_WIDTH;
	float font_descent = MOD(fd->descent);
	float file_name_height_size = round(fd->max_height + font_descent);

	vec4 file_switch_area_color = UI_BACKGROUND_COLOR;
	render_transparent_quad(file_switch_area_min_width, file_switch_area_min_height, file_switch_area_max_width, file_switch_area_max_height, &file_switch_area_color);

	s32 vertical_levels_num = get_number_of_main_text_vertical_levels();
	float main_text_width = floor((win_state.win_width - UI_LEFT_COLUMN_WIDTH - UI_RIGHT_COLUMN_WIDTH) /
		((float)vertical_levels_num));

	for (current_vertical_level = 0; current_vertical_level < vertical_levels_num; ++current_vertical_level)
	{
		interface_panel* first_level_panel = get_first_main_text_panel(current_vertical_level);
		auxiliar_panel = first_level_panel;
		float current_panel_width = get_main_panel_width_per_level(current_vertical_level);

		last_max_width = UI_LEFT_COLUMN_WIDTH + current_vertical_level * current_panel_width;

		do
		{
			file_switch_area_min_width = last_max_width;
			if (last_max_width != 0)	file_switch_area_max_width += spacement_per_files;	// if not first, space.

			u8* filepath = get_tid_file_name(auxiliar_panel->es->main_buffer_tid);
			u8* filename;

			if (filepath == null)
				filename = default_file_name;
			else
				filename = get_file_name_from_file_path(filepath);

			Font_RenderInInfo font_in_info = { 0 };
			Font_RenderOutInfo font_out_info;
			prerender_text(file_switch_area_min_width + file_name_width_spacement,
				file_switch_area_min_height + file_name_height_spacement,
				filename,
				hstrlen(filename),
				&font_out_info,
				&font_in_info);

			vec4 file_switch_area_item_color = UI_FILE_SWITCH_AREA_ITEM_BACKGROUND;

			// change BG color if this is the selected panel.
			if (auxiliar_panel->visible)
				file_switch_area_item_color = UI_FILE_SWITCH_AREA_SELECTED_ITEM_BACKGROUND;

			render_transparent_quad(file_switch_area_min_width,
				file_switch_area_min_height,
				font_out_info.exit_width + file_name_width_spacement,
				file_switch_area_min_height + file_name_height_size + 2 * file_name_height_spacement,
				&file_switch_area_item_color);

			vec4 file_switch_area_text_color = UI_FILE_SWITCH_AREA_TEXT_COLOR;
			render_text(file_switch_area_min_width + file_name_width_spacement,
				file_switch_area_min_height + file_name_height_spacement + font_descent,
				filename,
				hstrlen(filename),
				&file_switch_area_text_color);

			last_max_width = font_out_info.exit_width + file_name_width_spacement;

			auxiliar_panel = get_next_main_text_panel_by_level(auxiliar_panel);
		} while (auxiliar_panel != first_level_panel);
	}
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
