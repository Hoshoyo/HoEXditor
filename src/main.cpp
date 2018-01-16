#include "common.h"
#include "input.h"
#include "os_dependent.h"

#define HOGL_IMPLEMENT
#include <ho_gl.h>
#include "util.h"
#include <homath.h>
#include "font_rendering.h"
#include "renderer.h"
#include "editor.h"

global_variable Keyboard_State keyboard_state;
global_variable Mouse_State mouse_state;
global_variable Window_State win_state;
global_variable Application_State app;
global_variable Font_Info font_info;


#include "util.cpp"
#include "renderer.cpp"
#include "font_rendering.cpp"
#include "editor.cpp"

internal int init_opengl(Window_State* window_info)
{
	int PixelFormat;
	window_info->device_context = GetDC(window_info->window_handle);

	PIXELFORMATDESCRIPTOR pfd;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
	pfd.cDepthBits = 32;
	pfd.cColorBits = 24;
	pfd.iPixelType = PFD_TYPE_RGBA;

	PixelFormat = ChoosePixelFormat(window_info->device_context, &pfd);
	if (!SetPixelFormat(window_info->device_context, PixelFormat, &pfd)) {
		MessageBoxA(0, "Could not set a pixel format.\n", "Fatal Error", MB_ICONERROR);
		return -1;
	}

	HGLRC temp_context = wglCreateContext(window_info->device_context);
	if (!wglMakeCurrent(window_info->device_context, temp_context)) {
		MessageBoxA(0, "Could not create an OpenGL context.\n Make sure OpenGL compatible drivers are installed.", "Fatal Error", MB_ICONERROR);
		return -1;
	}

	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_FLAGS_ARB, 0,
		0
	};
	HGLRC(WINAPI* wglCreateContextAttribsARB)(HDC hDC, HGLRC hShareContext, int *attribList);
	wglCreateContextAttribsARB = (HGLRC(WINAPI*)(HDC, HGLRC, int *))wglGetProcAddress("wglCreateContextAttribsARB");
	if (wglCreateContextAttribsARB) {
		window_info->rendering_context = wglCreateContextAttribsARB(window_info->device_context, 0, attribs);
		if (window_info->rendering_context) {
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(temp_context);
			wglMakeCurrent(window_info->device_context, window_info->rendering_context);
			window_info->core_context = true;
		}
		else {
			OutputDebugStringA("OpenGL core context could not be created.\n");
			window_info->core_context = false;
		}
	}
	else {
		OutputDebugStringA("OpenGL core context could not be created.\n");
		window_info->core_context = false;
	}
	return 0;
}

LRESULT CALLBACK window_callback(HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg)
	{
	case WM_INITMENUPOPUP: {
		// reload plugins
		if (lparam == 3)
			printf("Plugins menu open\n");
	}break;
	case WM_COMMAND: {
		switch (wparam) {
		case F_COMMAND_OPEN: {
			wchar_t buffer[1024] = { 0 };

			OPENFILENAME fn = { 0 };
			fn.lStructSize = sizeof(OPENFILENAME);
			fn.lpstrFile = buffer;
			fn.nMaxFile = sizeof(buffer);

			GetOpenFileName(&fn);
			wprintf(L"Tried to open file %s\n", buffer);
		}break;
		case F_COMMAND_SAVE:
		case F_COMMAND_SAVEAS: {
			wchar_t buffer[1024] = { 0 };

			OPENFILENAME fn = { 0 };
			fn.lStructSize = sizeof(OPENFILENAME);
			fn.lpstrFile = buffer;
			fn.nMaxFile = sizeof(buffer);

			GetSaveFileName(&fn);
			wprintf(L"Tried to save file %s\n", buffer);
		}break;
		case F_COMMAND_EXIT: {
			app.running = false;
		}break;
		default: break;
		}
	}break;
	case WM_KILLFOCUS: {
		ZeroMemory(keyboard_state.key, MAX_KEYS);
	}break;
	case WM_CREATE: {
		app.file_menu = CreateMenu();
		AppendMenu(app.file_menu, MF_STRING, F_COMMAND_NEW, L"&New... \tCtrl+N");
		AppendMenu(app.file_menu, MF_STRING, F_COMMAND_OPEN, L"&Open...\tCtrl+O");
		AppendMenu(app.file_menu, MF_STRING, F_COMMAND_SAVE, L"&Save\tCtrl+S");
		AppendMenu(app.file_menu, MF_STRING, F_COMMAND_SAVEAS, L"Save &As...");
		AppendMenu(app.file_menu, MF_STRING, F_COMMAND_CLOSE, L"&Close\tCtrl+W");
		AppendMenu(app.file_menu, MF_MENUBREAK, 0, 0);
		AppendMenu(app.file_menu, MF_STRING, F_COMMAND_EXIT, L"E&xit");

		app.edit_menu = CreateMenu();
		AppendMenu(app.edit_menu, MF_STRING | MF_GRAYED, F_COMMAND_UNDO, L"&Undo\tCtrl+Z");
		AppendMenu(app.edit_menu, MF_STRING | MF_GRAYED, F_COMMAND_REDO, L"&Redo\tCtrl+Y");
		AppendMenu(app.edit_menu, MF_MENUBREAK, 0, 0);
		AppendMenu(app.edit_menu, MF_STRING | MF_GRAYED, F_COMMAND_CUT, L"Cu&t\tCtrl+X");
		AppendMenu(app.edit_menu, MF_STRING | MF_GRAYED, F_COMMAND_COPY, L"&Copy\tCtrl+C");
		AppendMenu(app.edit_menu, MF_STRING | MF_GRAYED, F_COMMAND_PASTE, L"&Paste\tCtrl+V");
		AppendMenu(app.edit_menu, MF_STRING, F_COMMAND_SEARCH, L"&Search\tCtrl+F");

		app.view_menu = CreateMenu();

		app.plugins_menu = CreateMenu();

		app.main_menu = CreateMenu();
		AppendMenu(app.main_menu, MF_POPUP, (UINT_PTR)app.file_menu, L"&File");
		AppendMenu(app.main_menu, MF_POPUP, (UINT_PTR)app.edit_menu, L"&Edit");
		AppendMenu(app.main_menu, MF_POPUP, (UINT_PTR)app.view_menu, L"&View");
		AppendMenu(app.main_menu, MF_POPUP, (UINT_PTR)app.plugins_menu, L"&Plugins");
		SetMenu(window, app.main_menu);


		app.subwindow = CreateWindow(L"static", L"HoExditor_Subwindow_OpenGL", WS_VISIBLE | WS_CHILD, 0, 0, 500, 500, window, 0, 0, 0);
		app.window_state.window_handle = app.subwindow;
		app.window_state.win_width = 500;
		app.window_state.win_height = 500;
		if (init_opengl(&app.window_state) == -1) {
			ExitProcess(-1);
		}
		hogl_init_gl_extensions();

		glClearColor(30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 1.0f);
	} break;
	case WM_CLOSE:
		app.running = false;
		break;
	case WM_CHAR:
		break;
	case WM_SIZE: {
		RECT client_rect;
		GetClientRect(window, &client_rect);
		int width = client_rect.right - client_rect.left;
		int height = client_rect.bottom - client_rect.top;
		if (app.subwindow) {
			BOOL res = SetWindowPos(app.subwindow, HWND_TOP, 0, 0, width, height, 0);
			win_state.win_width = width;
			win_state.win_height = height;
			if (!res) {
				OutputDebugStringA("Error resizing subwindow\n");
			} else {
				GetClientRect(app.subwindow, &client_rect);
				width = client_rect.right - client_rect.left;
				height = client_rect.bottom - client_rect.top;
				app.width = width;
				app.height = height;
				glViewport(0, 0, width, height);
			}
		}
		if (glClear) {
			editor_update_and_render();
			SwapBuffers(app.window_state.device_context);
		}
	} break;
	case WM_DROPFILES: {
		wchar_t buffer[512];
		HDROP hDrop = (HDROP)wparam;
		UINT ret = DragQueryFile(hDrop, 0, buffer, 512);
		POINT mouse_loc;
		DragQueryPoint(hDrop, &mouse_loc);
		DragFinish(hDrop);
		//handle_file_drop(mouse_loc.x, mouse_loc.y, buffer);
	}break;
	default:
		return DefWindowProc(window, msg, wparam, lparam);
	}
	return 0;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int cmd_show) {

	AllocConsole();
	FILE* pCout;
	freopen_s(&pCout, "CONOUT$", "w", stdout);
	
	WNDCLASSEX window_class = { 0 };
	window_class.cbSize = sizeof(WNDCLASSEX);
	window_class.style = 0;
	window_class.lpfnWndProc = window_callback;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = 0;
	window_class.hInstance = instance;
	window_class.hIcon = 0;
	window_class.hCursor = LoadCursor(0, IDC_ARROW);
	window_class.hbrBackground = (HBRUSH)COLOR_WINDOW;
	window_class.lpszMenuName = 0;
	window_class.lpszClassName = L"HoEXditor_Class";
	window_class.hIconSm = 0;

	if (!RegisterClassEx(&window_class)) {
		MessageBox(0, L"Error registering window class.", L"Fatal error", MB_ICONERROR);
		ExitProcess(-1);
	}

	HWND window = CreateWindowEx(
		WS_EX_ACCEPTFILES | WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
		window_class.lpszClassName, L"HoEXditor",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768, 0, 0, instance, 0);

	if (!window) {
		MessageBox(0, L"Error creating window", L"Fatal error", MB_ICONERROR);
		ExitProcess(-1);
	}

	win_state.win_width = 1024;
	win_state.win_height = 768;
	win_state.window_handle = window;
	win_state.device_context = 0;
	win_state.rendering_context = 0;

	u32 font_shader = shader_load(font_vshader, font_fshader);
	int error = font_load(&font_info, "C:\\Windows\\Fonts\\consola.ttf", 11, 1024);
	font_info.shader = font_shader;
	text_buffer_init(&font_info, 1024);
	init_immediate_quad_mode();

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	app.running = true;
	MSG msg;
	while (GetMessage(&msg, window, 0, 0) && app.running) {
		switch (msg.message) {
		case WM_MOUSEMOVE: {
			mouse_state.x = GET_X_LPARAM(msg.lParam);
			mouse_state.y = GET_Y_LPARAM(msg.lParam);
			//handle_mouse_move(mouse_state.x, mouse_state.y);
		} break;
		case WM_KEYDOWN: {
			bool ctrl_was_pressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
			switch (msg.wParam) {
			case 'O': {
				if (ctrl_was_pressed)
					SendMessage(window, WM_COMMAND, F_COMMAND_OPEN, 0);
			}break;
			case 'N': {
				if (ctrl_was_pressed)
					SendMessage(window, WM_COMMAND, F_COMMAND_NEW, 0);
			}break;
			case 'S': {
				if (ctrl_was_pressed)
					SendMessage(window, WM_COMMAND, F_COMMAND_SAVE, 0);
			}break;
			case 'A': {
				if (ctrl_was_pressed)
					SendMessage(window, WM_COMMAND, F_COMMAND_SAVEAS, 0);
			}break;
			case 'Z': {
				if (ctrl_was_pressed)
					SendMessage(window, WM_COMMAND, F_COMMAND_UNDO, 0);
			}break;
			case 'Y': {
				if (ctrl_was_pressed)
					SendMessage(window, WM_COMMAND, F_COMMAND_REDO, 0);
			}break;
			case 'X': {
				if (ctrl_was_pressed)
					SendMessage(window, WM_COMMAND, F_COMMAND_CUT, 0);
			}break;
			case 'C': {
				if (ctrl_was_pressed)
					SendMessage(window, WM_COMMAND, F_COMMAND_COPY, 0);
			}break;
			case 'V': {
				if (ctrl_was_pressed)
					SendMessage(window, WM_COMMAND, F_COMMAND_PASTE, 0);
			}break;
			case 'L': {
				if (ctrl_was_pressed) {
					//ModifyMenu(app.file_menu, F_COMMAND_NEW, MF_STRING, 0, L"&Novo...\tCtrl+N");
				}
			}break;
			case WM_LBUTTONDOWN: {
				int x = GET_X_LPARAM(msg.lParam);
				int y = GET_Y_LPARAM(msg.lParam);
				//handle_lmouse_down(x, y);
			} break;
			case WM_CHAR: {
				int key = msg.wParam;
				//handle_char_down(key);
			} break;
			}
		}break;
		default: {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}break;
		}

		editor_update_and_render();

		SwapBuffers(app.window_state.device_context);
	}

	ExitProcess(0);
}