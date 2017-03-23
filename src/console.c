#include "console.h"
#include "util.h"
#include "text_manager.h"
#include "input.h"
#include "interface.h"
#include "memory.h"

extern interface_panel console_view_panel;
extern interface_panel console_input_panel;

bool console_keep_log = false;

void update_console()
{
	if (console_keep_log)
	{
		Editor_State* es = ui_get_focused_editor();
		Editor_State* console_view_es = console_view_panel.es;
		Editor_State* console_input_es = console_input_panel.es;
		s64 cursor_offset = 0;
		u8 aux_str[64];
		s32 n;

		if (get_tid_text_size(console_view_es->main_buffer_tid) > 0)
			delete_text(console_view_es->main_buffer_tid, null, get_tid_text_size(console_view_es->main_buffer_tid), 0);

		insert_text(console_view_es->main_buffer_tid, "HoEXditor Console", sizeof("HoEXditor Console") - 1, cursor_offset);
		cursor_offset += sizeof("HoEXditor Console") - 1;

		insert_text(console_view_es->main_buffer_tid, "\n\nCursor offset: ", sizeof("\n\nCursor offset: ") - 1, cursor_offset);
		cursor_offset += sizeof("\n\nCursor offset: ") - 1;
		n = s64_to_str_base10(es->cursor_info.cursor_offset, aux_str);
		insert_text(console_view_es->main_buffer_tid, aux_str, n, cursor_offset);
		cursor_offset += n;

		insert_text(console_view_es->main_buffer_tid, "\nNext line count: ", sizeof("\nNext line count: ") - 1, cursor_offset);
		cursor_offset += sizeof("\nNext line count: ") - 1;
		n = s64_to_str_base10(es->cursor_info.next_line_count, aux_str);
		insert_text(console_view_es->main_buffer_tid, aux_str, n, cursor_offset);
		cursor_offset += n;

		insert_text(console_view_es->main_buffer_tid, "\nPrev line count: ", sizeof("\nPrev line count: ") - 1, cursor_offset);
		cursor_offset += sizeof("\nPrev line count: ") - 1;
		n = s64_to_str_base10(es->cursor_info.previous_line_count, aux_str);
		insert_text(console_view_es->main_buffer_tid, aux_str, n, cursor_offset);
		cursor_offset += n;

		insert_text(console_view_es->main_buffer_tid, "\nSnap cursor column: ", sizeof("\nSnap cursor column: ") - 1, cursor_offset);
		cursor_offset += sizeof("\nSnap cursor column: ") - 1;
		n = s64_to_str_base10(es->cursor_info.cursor_snaped_column, aux_str);
		insert_text(console_view_es->main_buffer_tid, aux_str, n, cursor_offset);
		cursor_offset += n;

		insert_text(console_view_es->main_buffer_tid, "\nCursor column: ", sizeof("\nCursor column: ") - 1, cursor_offset);
		cursor_offset += sizeof("\nCursor column: ") - 1;
		n = s64_to_str_base10(es->cursor_info.cursor_column, aux_str);
		insert_text(console_view_es->main_buffer_tid, aux_str, n, cursor_offset);
		cursor_offset += n;

		insert_text(console_view_es->main_buffer_tid, "\nCursor line: ", sizeof("\nCursor line: ") - 1, cursor_offset);
		cursor_offset += sizeof("\nCursor line: ") - 1;
		cursor_info cinfo = get_cursor_info(es->main_buffer_tid, es->cursor_info.cursor_offset);
		n = s64_to_str_base10(cinfo.line_number.lf, aux_str);
		insert_text(console_view_es->main_buffer_tid, aux_str, n, cursor_offset);
		cursor_offset += n;

		insert_text(console_view_es->main_buffer_tid, "\nText Size: ", sizeof("\nText Size: ") - 1, cursor_offset);
		cursor_offset += sizeof("\nText Size: ") - 1;
		n = s64_to_str_base10(get_tid_text_size(es->main_buffer_tid), aux_str);
		insert_text(console_view_es->main_buffer_tid, aux_str, n, cursor_offset);
		cursor_offset += n;

		insert_text(console_view_es->main_buffer_tid, "\nBuffer Valid Bytes: ", sizeof("\nBuffer Valid Bytes: ") - 1, cursor_offset);
		cursor_offset += sizeof("\nBuffer Valid Bytes: ") - 1;
		n = s64_to_str_base10(get_tid_valid_bytes(es->main_buffer_tid), aux_str);
		insert_text(console_view_es->main_buffer_tid, aux_str, n, cursor_offset);
		cursor_offset += n;

		insert_text(console_view_es->main_buffer_tid, "\nLast Line: ", sizeof("\nLast Line: ") - 1, cursor_offset);
		cursor_offset += sizeof("\nLast Line: ") - 1;
		n = s64_to_str_base10(es->cursor_info.last_line, aux_str);
		insert_text(console_view_es->main_buffer_tid, aux_str, n, cursor_offset);
		cursor_offset += n;
	}
}

s32 console_char_handler(s32 key)
{
	Editor_State* console_view_es = console_view_panel.es;
	Editor_State* console_input_es = console_input_panel.es;

	if (key == CARRIAGE_RETURN_KEY)
	{
		s32 command_size = get_tid_text_size(console_input_es->main_buffer_tid) * sizeof(u8);
		u8* command = halloc(command_size * sizeof(u8));
		delete_text(console_input_es->main_buffer_tid, command, command_size, 0);
		console_input_es->cursor_info.cursor_offset = 0;

		console_command cs_command = console_parse_command(command, command_size);
		console_execute_command(cs_command);
		console_free_parsed_command(cs_command);

		hfree(command);

		return 0;
	}

	return -1;
}

console_command console_parse_command(u8* command, s32 command_size)
{
	u32 aux, aux2, total;

	/* Gen ARGC and *ARGV */
	console_command cs_command;

	if (command_size > 0)
	{
		cs_command.argc = 1;

		// get argc
		for (aux = 0; aux < command_size; ++aux)
			if (command[aux] == ' ')
				++cs_command.argc;

		// alloc argv array.
		cs_command.argv = halloc(sizeof(u8*) * cs_command.argc);

		// next command init will be the pointer to the beggining of the next command that will be searched.
		u8* next_command_init = command;

		// get argvs
		for (aux = 0, total = 0; aux < cs_command.argc; ++aux)
		{
			aux2 = 0;
			// get current argv position
			while (next_command_init[aux2] != ' ' && total != command_size)
			{
				++aux2;
				++total;
			}

			cs_command.argv[aux] = halloc((aux2 + 1) * sizeof(u8)); // aux2 + 1 to add \0
			copy_string(cs_command.argv[aux], next_command_init, aux2); // copy command to argv position
			cs_command.argv[aux][aux2] = 0; // put \0
			next_command_init = next_command_init + aux2 + 1; // aux2 + 1 to skip space (' ')
			++total;  // ++total to skip space.
		}
	}
	else
	{
		cs_command.argc = 0;
		cs_command.argv = null;
	}

	/* Find out which Command was typed */
	if (cs_command.argc > 0 && cs_command.argv[0][0] == '/')
	{
		u8* command_text = cs_command.argv[0] + sizeof(u8); // skip '/'

		if (is_string_equal(command_text, CONSOLE_COMMAND_HELP))
			cs_command.type = HELP;
		else if (is_string_equal(command_text, CONSOLE_COMMAND_OPEN))
			cs_command.type = OPEN;
		else if (is_string_equal(command_text, CONSOLE_COMMAND_SAVE))
			cs_command.type = SAVE;
		else if (is_string_equal(command_text, CONSOLE_COMMAND_LOG))
			cs_command.type = LOG;
		else if (is_string_equal(command_text, CONSOLE_COMMAND_NEW_EMPTY_FILE))
			cs_command.type = NEW_EMPTY_FILE;
		else
			cs_command.type = UNDEFINED;
	}
	else
		cs_command.type = UNDEFINED;

	return cs_command;
}

internal void console_free_parsed_command(console_command cs_command)
{
	// Free *ARGV
	s32 aux;

	for (aux = 0; aux < cs_command.argc; ++aux)
		hfree(cs_command.argv[aux]);

	hfree(cs_command.argv);
}

void console_execute_command(console_command cs_command)
{
	switch (cs_command.type)
	{
	case HELP:
	{
		run_help_command(cs_command.argc, cs_command.argv);
	} break;
	case OPEN:
	{
		run_open_command(cs_command.argc, cs_command.argv);
	} break;
	case LOG:
	{
		run_log_command(cs_command.argc, cs_command.argv);
	} break;
	case SAVE:
	{
		run_save_command(cs_command.argc, cs_command.argv);
	} break;
	case NEW_EMPTY_FILE:
	{
		run_new_empty_file_command(cs_command.argc, cs_command.argv);
	} break;
	default:
	{
		run_default_command(cs_command.argc, cs_command.argv);
	} break;
	}
}

void run_help_command(s32 argc, u8* argv[])
{
	Editor_State* console_view_es = console_view_panel.es;
	u8 help_text[] = "Commands: /new, /help, /log, /save <PATH>, /open <PATH>";
	s32 help_text_size = sizeof("Commands: /new, /help, /log, /save <PATH>, /open <PATH>") - 1;

	if (get_tid_text_size(console_view_es->main_buffer_tid) > 0)
		delete_text(console_view_es->main_buffer_tid, null, get_tid_text_size(console_view_es->main_buffer_tid), 0);

	insert_text(console_view_es->main_buffer_tid, help_text, help_text_size, 0);
}

void run_open_command(s32 argc, u8* argv[])
{
	Editor_State* es = ui_get_focused_editor();
	u8 success_text[] = "File opened successfully";
	s32 success_text_size = sizeof("File opened successfully") - 1;
	u8 unknown_error_text[] = "Error: Unknown Error.";
	s32 unknown_error_text_size = sizeof("Error: Unknown Error.") - 1;
	u8 argument_error_text[] = "Error parsing command. Usage: /open <PATH>";
	s32 argument_error_text_size = sizeof("Error parsing command. Usage: /open <PATH>") - 1;
	u8 not_found_error_text[] = "Could not Open File: Invalid path.";
	s32 not_found_error_text_size = sizeof("Could not Open File: Invalid path.") - 1;

	Editor_State* console_view_es = console_view_panel.es;

	if (get_tid_text_size(console_view_es->main_buffer_tid) > 0)
		delete_text(console_view_es->main_buffer_tid, null, get_tid_text_size(console_view_es->main_buffer_tid), 0);

	if (argc > 1)
	{
		if (does_path_exist(argv[1]))
		{
			if (!ui_open_file(false, argv[1]))
			{
				update_buffer(es);
				insert_text(console_view_es->main_buffer_tid, success_text, success_text_size, 0);
			}
			else
				insert_text(console_view_es->main_buffer_tid, unknown_error_text, unknown_error_text_size, 0);
		}
		else
			insert_text(console_view_es->main_buffer_tid, not_found_error_text, not_found_error_text_size, 0);
	}
	else
		insert_text(console_view_es->main_buffer_tid, argument_error_text, argument_error_text_size, 0);
}

void run_log_command(s32 argc, u8* argv[])
{
	Editor_State* console_view_es = console_view_panel.es;
	console_keep_log = !console_keep_log;

	// if log was turned off
	if (!console_keep_log)
	{
		u8 log_turned_off_text[] = "/log turned off.";
		s32 log_turned_off_text_size = sizeof("/log turned off.") - 1;

		if (get_tid_text_size(console_view_es->main_buffer_tid) > 0)
			delete_text(console_view_es->main_buffer_tid, null, get_tid_text_size(console_view_es->main_buffer_tid), 0);

		insert_text(console_view_es->main_buffer_tid, log_turned_off_text, log_turned_off_text_size, 0);
	}
}

void run_save_command(s32 argc, u8* argv[])
{
	Editor_State* console_view_es = console_view_panel.es;
	Editor_State* es = ui_get_focused_editor();
	u8 success_text[] = "File saved successfully";
	s32 success_text_size = sizeof("File saved successfully") - 1;
	u8 unknown_error_text[] = "Error: Unknown Error.";
	s32 unknown_error_text_size = sizeof("Error: Unknown Error.") - 1;
	u8 argument_error_text[] = "Error parsing command. Usage: /save <PATH>";
	s32 argument_error_text_size = sizeof("Error parsing command. Usage: /save <PATH>") - 1;
	u8 not_found_error_text[] = "Could not save File: Invalid path.";
	s32 not_found_error_text_size = sizeof("Could not save File: Invalid path.") - 1;

	if (get_tid_text_size(console_view_es->main_buffer_tid) > 0)
		delete_text(console_view_es->main_buffer_tid, null, get_tid_text_size(console_view_es->main_buffer_tid), 0);

	if (argc > 1)
	{
		u8* path = remove_file_name_from_file_path(argv[1]);

		if (does_path_exist(path))
		{
			if (!ui_save_file(argv[1]))
				insert_text(console_view_es->main_buffer_tid, success_text, success_text_size, 0);
			else
				insert_text(console_view_es->main_buffer_tid, unknown_error_text, unknown_error_text_size, 0);
		}
		else
			insert_text(console_view_es->main_buffer_tid, not_found_error_text, not_found_error_text_size, 0);

		hfree(path);
	}
	else
		insert_text(console_view_es->main_buffer_tid, argument_error_text, argument_error_text_size, 0);
}

void run_new_empty_file_command(s32 argc, u8* argv[])
{
	Editor_State* console_view_es = console_view_panel.es;
	Editor_State* es = ui_get_focused_editor();
	u8 success_text[] = "Empty file created successfully.";
	s32 success_text_size = sizeof("Empty file opened successfully") - 1;
	u8 unknown_error_text[] = "Error: Unknown Error.";
	s32 unknown_error_text_size = sizeof("Error: Unknown Error.") - 1;

	if (get_tid_text_size(console_view_es->main_buffer_tid) > 0)
		delete_text(console_view_es->main_buffer_tid, null, get_tid_text_size(console_view_es->main_buffer_tid), 0);

	if (!ui_open_file(true, null))
	{
		update_buffer(es);
		insert_text(console_view_es->main_buffer_tid, success_text, success_text_size, 0);
	}
	else
		insert_text(console_view_es->main_buffer_tid, unknown_error_text, unknown_error_text_size, 0);
}

void run_default_command(s32 argc, u8* argv[])
{
	Editor_State* console_view_es = console_view_panel.es;
	u8 unknown_command_text[] = "u nuts? this is an invalid command... try /help";
	s32 unknown_command_text_size = sizeof("u nuts? this is an invalid command... try /help") - 1;

	if (get_tid_text_size(console_view_es->main_buffer_tid) > 0)
		delete_text(console_view_es->main_buffer_tid, null, get_tid_text_size(console_view_es->main_buffer_tid), 0);

	insert_text(console_view_es->main_buffer_tid, unknown_command_text, unknown_command_text_size, 0);
}
