#ifndef HOHEX_OS_DEPENDENT_H
#define HOHEX_OS_DEPENDENT_H

#if defined(_WIN64)
typedef struct {
	HWND window_handle;
	s32 win_width, win_height;
	WINDOWPLACEMENT g_wpPrev;
	HDC device_context;
	HGLRC rendering_context;
	bool core_context;
} Window_State;

typedef enum {
	F_COMMAND_NONE = 0,
	F_COMMAND_OPEN,
	F_COMMAND_CLOSE,
	F_COMMAND_NEW,
	F_COMMAND_SAVE,
	F_COMMAND_SAVEAS,
	F_COMMAND_EXIT,

	F_COMMAND_UNDO,
	F_COMMAND_REDO,
	F_COMMAND_CUT,
	F_COMMAND_PASTE,
	F_COMMAND_COPY,
	F_COMMAND_SEARCH,

	F_COMMAND_NUMBER
} MenuCommands;

typedef struct {
	bool running;

	Window_State window_state;

	s32 width;
	s32 height;

	HMENU main_menu;
	HMENU file_menu;
	HMENU edit_menu;
	HMENU view_menu;
	HMENU plugins_menu;

	HWND  subwindow;
} Application_State;
#endif

s32 open_clipboard();
s32 get_clipboard_content(u8** content);
s32 set_clipboard_content(u8* content, u64 content_size);
s32 close_clipboard();

s32 write_file();

#endif
