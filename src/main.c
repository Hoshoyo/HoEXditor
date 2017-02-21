#include "common.h"

#define HOGL_IMPLEMENT
#include "ho_gl.h"
#include "memory.h"
#include "util.h"
#include "font_rendering.h"
#include "math/homath.h"
#include "editor.h"
#include "text_manager.h"

#if defined(_WIN64)

typedef struct {
	s32 x, y;
	s32 x_left, y_left;
	s32 wheel_value;

	bool left_down;
	bool right_down;
	bool middle_down;

	bool is_captured;
} Mouse_State;

#define MAX_KEYS 1024
typedef struct {
	bool key[MAX_KEYS];
} Keyboard_State;

extern Mouse_State mouse_state = { 0 };		// global
extern Keyboard_State keyboard_state = { 0 };	// global

typedef struct {
	HWND window_handle;
	LONG win_width, win_height;
	WINDOWPLACEMENT g_wpPrev;
	HDC device_context;
	HGLRC rendering_context;
} Window_State;
Window_State win_state = {0};

bool state = false;

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
		print("Attempted to drop file (%s) at mouse location {%d, %d}.\n", buffer, mouse_loc.x, mouse_loc.y);
	}break;
	default:
		return DefWindowProc(window, msg, wparam, lparam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int cmd_show)
{
#if 1
	AllocConsole();
	FILE* pCout;
	freopen_s(&pCout, "CONOUT$", "w", stdout);
#endif
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

	// alloc console
#if 1
	AllocConsole();
	FILE* pCout;
	freopen_s(&pCout, "CONOUT$", "w", stdout);
#endif

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
	
	init_editor();
	

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
					keyboard_state.key[key] = true;
					handle_key_down(key);
					if (key == 'Q') state = !state;
				}break;
				case WM_KEYUP: {
					int key = msg.wParam;
					keyboard_state.key[key] = false;
				}break;
				case WM_MOUSEMOVE: {
					mouse_state.x = GET_X_LPARAM(msg.lParam);
					mouse_state.y = GET_Y_LPARAM(msg.lParam);
				} break;
				case WM_LBUTTONDOWN: {
					int x = GET_X_LPARAM(msg.lParam);
					int y = GET_Y_LPARAM(msg.lParam);
					print("x: %d, y: %d\n", x, y);
				}break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		render_editor();
#if 0
		{
			vec4 debug_yellow = (vec4) { 1.0f, 1.0f, 0.0f, 0.5f };
			glUniform4fv(font_rendering.font_color_uniform_location, 1, (GLfloat*)&debug_yellow);
			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);

			glBegin(GL_LINES);

			glVertex3f(1.0f, win_state.win_height - font_rendering.max_height - 5.0f, 0.0f);
			glVertex3f(1.0f, 1.0f, 0.0f);

			glVertex3f(win_state.win_width - 1.0f, win_state.win_height - font_rendering.max_height - 5.0f, 0.0f);
			glVertex3f(win_state.win_width - 1.0f, 1.0f, 0.0f);

			glVertex3f(1.0f, win_state.win_height - font_rendering.max_height - 5.0f, 0.0f);
			glVertex3f(win_state.win_width - 1.0f, win_state.win_height - font_rendering.max_height - 5.0f, 0.0f);

			glVertex3f(1.0f, 1.0f, 0.0f);
			glVertex3f(win_state.win_width - 1.0f, 1.0f, 0.0f);

			glEnd();

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
#endif
		SwapBuffers(win_state.device_context);
	}

	return 0;
}

#elif defined(_LINUX)
#error OS not yet supported.
#endif
