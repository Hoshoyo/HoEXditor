#ifndef HOHEX_OS_DEPENDENT_H
#define HOHEX_OS_DEPENDENT_H

#include "common.h"

#if defined(_WIN64)
#include <windows.h>
typedef struct {
	HWND window_handle;
	LONG win_width, win_height;
	WINDOWPLACEMENT g_wpPrev;
	HDC device_context;
	HGLRC rendering_context;
} Window_State;
#endif

s32 open_clipboard();
s32 get_clipboard_content(u8** content);
s32 set_clipboard_content(u8* content, u64 content_size);
s32 close_clipboard();

s32 write_file();

#endif
