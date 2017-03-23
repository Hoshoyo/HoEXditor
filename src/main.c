#include "common.h"

#define HOGL_IMPLEMENT
#include "ho_gl.h"
#include "memory.h"
#include "util.h"
#include "font_rendering.h"
#include "math/homath.h"
#include "editor.h"
#include "text_manager.h"
#include "text_events.h"
#include "input.h"
#include "os_dependent.h"
#include "interface.h"

#if defined(_WIN64)

Keyboard_State keyboard_state = { 0 };		// global
Mouse_State mouse_state = { 0 };

extern Editor_State* focused_editor_state;
extern Window_State win_state;

LRESULT CALLBACK WndProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_KILLFOCUS: {
		ZeroMemory(keyboard_state.key, MAX_KEYS);
	}break;
	case WM_CREATE: break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SYSKEYDOWN:
		break;
	case WM_SYSKEYUP:
		break;
	case WM_CHAR:
		break;
	case WM_SIZE: {
		RECT r;
		GetClientRect(window, &r);
		win_state.win_width = r.right - r.left;
		win_state.win_height = r.bottom - r.top;
		glViewport(0, 0, win_state.win_width, win_state.win_height);
		update_font((float)win_state.win_width, (float)win_state.win_height);
		if (is_interface_initialized)
			prerender_top_menu();

		// @TODO THIS SHOULD BE DONE: REDRAW HERE
		// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// SwapBuffers(device_context);
	} break;
	case WM_DROPFILES: {
		char buffer[512];
		HDROP hDrop = (HDROP)wparam;
		UINT ret = DragQueryFile(hDrop, 0, buffer, 512);
		POINT mouse_loc;
		DragQueryPoint(hDrop, &mouse_loc);
		DragFinish(hDrop);
		handle_file_drop(mouse_loc.x, mouse_loc.y, buffer);
	}break;
	default:
		return DefWindowProc(window, msg, wparam, lparam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int cmd_show)
{
	make_arena(MEGABYTE(1));

	WNDCLASSEX window_class;
	window_class.cbSize = sizeof(WNDCLASSEX);
	window_class.style = 0;// CS_HREDRAW | CS_VREDRAW;	// @todo do i need this?
	window_class.lpfnWndProc = WndProc;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = 0;
	window_class.hInstance = instance;
	window_class.hIcon = LoadIcon(instance, MAKEINTRESOURCE(NULL));
	window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_class.hbrBackground = 0;
	window_class.lpszMenuName = NULL;
	window_class.lpszClassName = "HoEXditor_Class";
	window_class.hIconSm = NULL;

	if (!RegisterClassEx(&window_class)) error_fatal("Error creating window class.\n", 0);

	// alloc console
#if 1
	AllocConsole();
	FILE* pCout;
	freopen_s(&pCout, "CONOUT$", "w", stdout);
#endif

	u32 window_style_exflags = WS_EX_ACCEPTFILES | WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	u32 window_style_flags = WS_OVERLAPPEDWINDOW;
	// Note: Client area must be correct, so windows needs to get the WindowRect
	// area depending on the style of the window
	RECT window_rect = {0};
	window_rect.right = 1024;
	window_rect.bottom = 768;
	AdjustWindowRectEx(&window_rect, window_style_flags, FALSE, window_style_exflags);

	win_state.g_wpPrev.length = sizeof(WINDOWPLACEMENT);
	win_state.win_width = window_rect.right - window_rect.left;
	win_state.win_height = window_rect.bottom - window_rect.top;

	win_state.window_handle = CreateWindowExA(
		window_style_exflags,
		window_class.lpszClassName,
		"HoEXditor",
		window_style_flags,
		CW_USEDEFAULT, CW_USEDEFAULT,
		win_state.win_width, win_state.win_height, NULL, NULL, instance, NULL
	);
	if (!win_state.window_handle) error_fatal("Error criating window context.\n", 0);

	ShowWindow(win_state.window_handle, cmd_show);
	UpdateWindow(win_state.window_handle);

	init_opengl(win_state.window_handle, &win_state.device_context, &win_state.rendering_context);
	wglSwapIntervalEXT(1);		// Enable Vsync

	bool running = true;
	MSG msg;

	// Track mouse events
	TRACKMOUSEEVENT mouse_event = {0};
	mouse_event.cbSize = sizeof(mouse_event);
	mouse_event.dwFlags = TME_LEAVE;
	mouse_event.dwHoverTime = HOVER_DEFAULT;
	mouse_event.hwndTrack = win_state.window_handle;

	init_interface();

	while(running){
		TrackMouseEvent(&mouse_event);
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0){
			if(msg.message == WM_QUIT){
				running = false;
				continue;
			}
			switch(msg.message){
				case WM_KEYDOWN: {
					int key = msg.wParam;
					int mod = msg.lParam;
					handle_key_down(key, mod);
				} break;
				case WM_KEYUP: {
					int key = msg.wParam;
					handle_key_up(key);
				} break;
				case WM_MOUSEMOVE: {
					mouse_state.x = GET_X_LPARAM(msg.lParam);
					mouse_state.y = GET_Y_LPARAM(msg.lParam);
					handle_mouse_move(mouse_state.x, mouse_state.y);
				} break;
				case WM_LBUTTONDOWN: {
					int x = GET_X_LPARAM(msg.lParam);
					int y = GET_Y_LPARAM(msg.lParam);
					handle_lmouse_down(x, y);
				} break;
				case WM_CHAR: {
					int key = msg.wParam;
					handle_char_down(key);
				} break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (is_interface_initialized) render_interface();

		SwapBuffers(win_state.device_context);
	}

	return 0;
}

#elif defined(_LINUX)
#error OS not yet supported.
#endif
