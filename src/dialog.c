#include "dialog.h"
#include "os_dependent.h"
#include "memory.h"
#include "input.h"
#include "text_manager.h"
#include "interface_definitions.h"
#include "font_rendering.h"

extern Window_State win_state;

ui_dialog* _dl_dialogs[MAX_NUM_DIALOGS] = { null };
u32 _dl_dialogs_num = 0;

ui_dialog* create_dialog
	(float x,
	float y,
	float height,
	float width,
	float ratio,
	vec4 view_font_color,
	vec4 view_cursor_color,
	vec4 view_background_color,
	vec4 input_font_color,
	vec4 input_cursor_color,
	vec4 input_background_color,
	bool dark_background_when_rendering,
	void(*dialog_callback)(u8*))
{
	ui_dialog* dialog = halloc(sizeof(ui_dialog));
	dialog->view_panel = halloc(sizeof(interface_panel));
	dialog->view_panel->es = halloc(sizeof(Editor_State));
	create_tid(&dialog->view_panel->es->main_buffer_tid, false);

	init_editor_state(dialog->view_panel->es);
	dialog->view_panel->es->font_color = view_font_color;
	dialog->view_panel->es->cursor_color = view_cursor_color;
	dialog->view_panel->es->line_number_color = (vec4) { 0, 0, 0, 0 };
	dialog->view_panel->es->parent_panel = dialog->view_panel;

	create_real_buffer(dialog->view_panel->es->main_buffer_tid, UI_DIALOG_VIEW_BUFFER_SIZE);
	setup_view_buffer(dialog->view_panel->es, 0, UI_DIALOG_VIEW_BUFFER_SIZE, true);

	dialog->view_panel->background_color = view_background_color;
	dialog->view_panel->visible = false;
	dialog->view_panel->is_main_text_panel = false;
	dialog->view_panel->main_text_panel_vertical_level = 0;

	dialog->input_panel = halloc(sizeof(interface_panel));
	dialog->input_panel->es = halloc(sizeof(Editor_State));
	create_tid(&dialog->input_panel->es->main_buffer_tid, false);

	init_editor_state(dialog->input_panel->es);
	dialog->input_panel->es->font_color = input_font_color;
	dialog->input_panel->es->cursor_color = input_cursor_color;
	dialog->input_panel->es->line_number_color = (vec4) { 0, 0, 0, 0 };
	dialog->input_panel->es->parent_panel = dialog->input_panel;
	dialog->input_panel->es->individual_char_handler = dialog_char_handler;

	create_real_buffer(dialog->input_panel->es->main_buffer_tid, UI_DIALOG_INPUT_BUFFER_SIZE);
	setup_view_buffer(dialog->input_panel->es, 0, UI_DIALOG_INPUT_BUFFER_SIZE, true);

	dialog->input_panel->background_color = input_background_color;
	dialog->input_panel->visible = false;
	dialog->input_panel->is_main_text_panel = false;
	dialog->input_panel->main_text_panel_vertical_level = 0;

	dialog->x = x;
	dialog->y = y;
	dialog->height = height;
	dialog->width = width;
	dialog->ratio = ratio;
	dialog->dark_background_when_rendering = dark_background_when_rendering;
	dialog->dialog_callback = dialog_callback;

	// Insert dialog into _dl_dialogs array.
	_dl_dialogs[_dl_dialogs_num++] = dialog;

	return dialog;
}

void destroy_dialog(ui_dialog* dialog)
{
	u32 aux = 0;

	while (dialog != _dl_dialogs[aux]) ++aux;

	for (; aux < _dl_dialogs_num - 1; ++aux)
		_dl_dialogs[aux] = _dl_dialogs[aux + 1];

	_dl_dialogs[aux] = null;

	--_dl_dialogs_num;

	free_interface_panel(dialog->input_panel);
	free_interface_panel(dialog->view_panel);
	hfree(dialog);
}

void update_dialogs()
{
	u32 aux;

	for (aux = 0; aux < _dl_dialogs_num; ++aux)
	{
		ui_dialog* dialog = _dl_dialogs[aux];

		float view_height = round(dialog->height * dialog->ratio);
		float input_height = round(dialog->height * (1 - dialog->ratio));

		dialog->view_panel->x = dialog->x;
		dialog->view_panel->y = dialog->y + input_height;
		dialog->view_panel->width = dialog->width;
		dialog->view_panel->height = view_height;

		dialog->view_panel->es->container.left_padding = dialog->view_panel->x;
		dialog->view_panel->es->container.right_padding = win_state.win_width - (dialog->view_panel->x + dialog->view_panel->width);
		dialog->view_panel->es->container.top_padding = win_state.win_height - (dialog->view_panel->y + dialog->view_panel->height);
		dialog->view_panel->es->container.bottom_padding = dialog->view_panel->y;

		dialog->input_panel->x = dialog->x;
		dialog->input_panel->y = dialog->y;
		dialog->input_panel->width = dialog->width;
		dialog->input_panel->height = input_height;

		dialog->input_panel->es->container.left_padding = dialog->input_panel->x;
		dialog->input_panel->es->container.right_padding = win_state.win_width - (dialog->input_panel->x + dialog->input_panel->width);
		dialog->input_panel->es->container.top_padding = win_state.win_height - (dialog->input_panel->y + dialog->input_panel->height);
		dialog->input_panel->es->container.bottom_padding = dialog->input_panel->y;

		if (dialog->view_panel->visible)
			dialog->view_panel->es->render = true;
		else
			dialog->view_panel->es->render = false;

		if (dialog->input_panel->visible)
			dialog->input_panel->es->render = true;
		else
			dialog->input_panel->es->render = false;


		update_container(dialog->view_panel->es);
		update_container(dialog->input_panel->es);
	}
}

void change_view_content(ui_dialog* dialog, u8* text, u64 text_size)
{
	text_id tid = dialog->view_panel->es->main_buffer_tid;
	if (tid.id > 0) delete_text(tid, null, get_tid_text_size(tid), 0);
	insert_text(tid, text, text_size, 0);
}

void change_dialog_visibility(ui_dialog* dialog, bool visible)
{
	dialog->view_panel->visible = visible;
	dialog->input_panel->visible = visible;
}

void render_dialogs()
{
	u32 aux;
	for (aux = 0; aux < _dl_dialogs_num; ++aux)
	{
		if (_dl_dialogs[aux]->view_panel->visible &&
			_dl_dialogs[aux]->input_panel->visible &&
			_dl_dialogs[aux]->dark_background_when_rendering)
		{
			// Render Dark Background
			vec4 color = (vec4) { 0.0f, 0.0f, 0.0f, 0.7f };
			render_transparent_quad(0, 0, win_state.win_width, win_state.win_height, &color);
		}
		render_interface_panel(_dl_dialogs[aux]->view_panel);
		render_interface_panel(_dl_dialogs[aux]->input_panel);
	}
}

s32 dialog_char_handler(Editor_State* es, s32 key)
{
	if (key == CARRIAGE_RETURN_KEY)
	{
		s32 text_size = get_tid_text_size(es->main_buffer_tid);
		u8* text = halloc(text_size * sizeof(u8) + 1 * sizeof(u8)); // 1 more to \0
		delete_text(es->main_buffer_tid, text, text_size, 0);
		text[text_size] = 0;
		es->cursor_info.cursor_offset = 0;

		// Find the related dialog.
		u32 aux;
		for (aux = 0; aux < _dl_dialogs_num; ++aux)
			if (_dl_dialogs[aux]->input_panel->es == es)
				break;

		_dl_dialogs[aux]->dialog_callback(text);
		hfree(text);

		return 0;
	}

	return -1;
}