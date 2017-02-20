#include "editor.h"
#include "math/homath.h"
#include "font_rendering.h"
#include "Psapi.h"

Editor_State editor_state = {0};

typedef struct {
	HWND window_handle;
	LONG win_width, win_height;
	WINDOWPLACEMENT g_wpPrev;
	HDC device_context;
	HGLRC rendering_context;
} Window_State;

extern Window_State win_state;
char text_arr[1024];

void init_editor()
{
	editor_state.cursor = 0;
	for (int i = 0; i < 1023; ++i) {
		text_arr[i] = 'F';
	}
	text_arr[1023] = 'G';
	//char font[] = "res/LiberationMono-Regular.ttf";
	//char font[] = "c:/windows/fonts/times.ttf";
	char font[] = "c:/windows/fonts/consola.ttf";
	s32 font_size = 18;	// @TEMPORARY @TODO make this configurable
	init_font(font, font_size, win_state.win_width, win_state.win_height);
}

void render_editor()
{
	vec4 font_color = (vec4) { 0.8f, 0.8f, 0.8f, 1.0f };

	//glEnable(GL_SCISSOR_TEST);
	//glScissor(1.0f, 1.0f, win_state.win_width - 1.0f, win_state.win_height - font_rendering.max_height - 5.0f);
	
	float off = font_rendering.max_height;
	Font_Render_Info render_info;
	render_info.cursor_position = editor_state.cursor;
	int num_rendered = 0;
	for (int i = 0; num_rendered < 1023; ++i) {
		num_rendered += render_text(0.0f, win_state.win_height - font_rendering.max_height - off, text_arr + num_rendered, 1023 - num_rendered, win_state.win_width, &font_color, &render_info);
		off += font_rendering.max_height;
	}
	vec4 cursor_color = (vec4) { 0.5f, 0.9f, 0.85f, 0.5f };
	int x0, x1, y0, y1;
	stbtt_GetCodepointBox(&font_rendering.font_info, text_arr[editor_state.cursor], &x0, &y0, &x1, &y1);
	float Fwidth = (x1 * font_rendering.downsize + x0 * font_rendering.downsize);
	render_transparent_quad(render_info.advance_x_cursor, win_state.win_height - (font_rendering.max_height * editor_state.line), Fwidth + render_info.advance_x_cursor, win_state.win_height - (font_rendering.max_height * (editor_state.line - 1.0f)), &cursor_color);
	PROCESS_MEMORY_COUNTERS pmcs;
	GetProcessMemoryInfo(GetCurrentProcess(), &pmcs, sizeof(pmcs));
	char buffer[64];
	int wrtn = s64_to_str_base10(pmcs.PagefileUsage, buffer);

	render_text(2.0f, -font_rendering.descent, "Memory usage: ", sizeof "Memory usage: " - 1, win_state.win_width, &font_color, &render_info);
	wrtn = render_text(render_info.last_x, -font_rendering.descent, buffer, wrtn, win_state.win_width, &font_color, &render_info);
	render_text(render_info.last_x, -font_rendering.descent, " bytes", sizeof " bytes" - 1, win_state.win_width, &font_color, 0);
}

void handle_key_down(s32 key)
{
	if (key == 'R') recompile_font_shader();
	if (key == 'F') debug_toggle_font_boxes();
	if (key == VK_RIGHT) editor_state.cursor++;
	if (key == VK_LEFT) editor_state.cursor = CLAMP_DOWN(editor_state.cursor - 1, 0);
	if (key == VK_UP) editor_state.line = CLAMP_DOWN(editor_state.line - 1, 1);
	if (key == VK_DOWN) editor_state.line++;

	if (key == 'G') {
		release_font();
		u8 font[] = "c:/windows/fonts/times.ttf";
		load_font(font, 20);
	}
	if (key == 'H') {
		release_font();
		u8 font[] = "res/LiberationMono-Regular.ttf";
		load_font(font, 20);
	}
	if (key == 'J') {
		release_font();
		u8 font[] = "c:/windows/fonts/consola.ttf";
		load_font(font, 20);
	}
}