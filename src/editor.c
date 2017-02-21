#include "editor.h"
#include "math/homath.h"
#include "font_rendering.h"
#include "text_manager.h"
#include "Psapi.h"

#define DEBUG 1

Editor_State editor_state = {0};

typedef struct {
	HWND window_handle;
	LONG win_width, win_height;
	WINDOWPLACEMENT g_wpPrev;
	HDC device_context;
	HGLRC rendering_context;
} Window_State;

extern Window_State win_state;

#define MAX_KEYS 1024
typedef struct {
	bool key[MAX_KEYS];
} Keyboard_State;
extern Keyboard_State keyboard_state;

void init_editor()
{
	//char font[] = "res/LiberationMono-Regular.ttf";
	char font[] = "c:/windows/fonts/times.ttf";
	//char font[] = "c:/windows/fonts/consola.ttf";
	s32 font_size = 20;	// @TEMPORARY @TODO make this configurable
	init_font(font, font_size, win_state.win_width, win_state.win_height);

	init_text_api("./res/dummy.txt");
	//end_text_api();
	//init_text_api("./res/m79.txt");

	editor_state.cursor = 0;
	editor_state.buffer_size = _tm_text_size;
	editor_state.buffer = get_text_buffer(_tm_text_size, 0);
	//end_text_api();

	Text_Container* container = &editor_state.container;
	container->left_padding = 2.0f;
	container->right_padding = 2.0f;
	container->bottom_padding = 2.0f;
	container->top_padding = 20.0f;
	update_container(container);
}

void update_container(Text_Container* container)
{
	container->minx = container->left_padding;
	container->maxx = win_state.win_width - container->right_padding;
	container->miny = container->bottom_padding;
	container->maxy = win_state.win_height - container->top_padding;
}

//glEnable(GL_SCISSOR_TEST);
//glScissor(1.0f, 1.0f, win_state.win_width - 1.0f, win_state.win_height - font_rendering.max_height - 5.0f);
void render_editor()
{
	vec4 font_color = (vec4) { 0.8f, 0.8f, 0.8f, 1.0f };
	Font_Render_Info render_info;
	render_info.cursor_position = editor_state.cursor;
	render_info.current_line = 0;
	int written = 0;
	update_container(&editor_state.container);

	// render text in the buffer
	float offset_y = 0;
	while (written < editor_state.buffer_size) {
		render_info.current_line++;
		render_info.in_offset = written;
		written += render_text( editor_state.container.minx, editor_state.container.maxy - font_rendering.max_height + offset_y,
								editor_state.buffer + written, editor_state.buffer_size - written,
								editor_state.container.maxx, &font_color, &render_info);
		offset_y -= font_rendering.max_height;
	}

	// render cursor overtop
	vec4 cursor_color = (vec4) { 0.5f, 0.9f, 0.85f, 0.5f };

	float min_x = render_info.advance_x_cursor;
	float max_x = min_x + render_info.cursor_char_width;
	if (render_info.cursor_char_width == 0) {
		max_x = render_info.last_x;
	}

	float min_y = editor_state.container.maxy - ((font_rendering.max_height) * render_info.cursor_line) + font_rendering.descent;
	float max_y = editor_state.container.maxy - ((font_rendering.max_height) * (render_info.cursor_line - 1)) - (font_rendering.max_height - font_rendering.ascent);
	render_transparent_quad(min_x, min_y,
							max_x, max_y,
							&cursor_color);

#if 1
	PROCESS_MEMORY_COUNTERS pmcs;
	GetProcessMemoryInfo(GetCurrentProcess(), &pmcs, sizeof(pmcs));
	char buffer[64];
	int wrtn = s64_to_str_base10(pmcs.PagefileUsage, buffer);
	render_text(2.0f, -font_rendering.descent, "Memory usage: ", sizeof "Memory usage: " - 1, win_state.win_width, &font_color, &render_info);
	wrtn = render_text(render_info.last_x, -font_rendering.descent, buffer, wrtn, win_state.win_width, &font_color, &render_info);
	render_text(render_info.last_x, -font_rendering.descent, " bytes", sizeof " bytes" - 1, win_state.win_width, &font_color, &render_info);

	render_text(render_info.last_x, -font_rendering.descent, " cursor_pos: ", sizeof " cursor_pos: " - 1, win_state.win_width, &font_color, &render_info);
	wrtn = s64_to_str_base10(editor_state.cursor, buffer);
	wrtn = render_text(render_info.last_x, -font_rendering.descent, buffer, wrtn, win_state.win_width, &font_color, 0);
#endif
}

void handle_key_down(s32 key)
{
	s64 cursor = editor_state.cursor;

#if DEBUG
	if (key == 'R') recompile_font_shader();
	if (key == 'F') debug_toggle_font_boxes();

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
#endif

	if (key == VK_RIGHT) {
		s64 increment = 1;
		if (keyboard_state.key[17]) increment = 8;
		editor_state.cursor = MIN(editor_state.cursor + increment, editor_state.buffer_size - 1);
	}
	if (key == VK_LEFT) {
		s64 increment = 1;
		if (keyboard_state.key[17]) increment = 8;
		editor_state.cursor = MAX(editor_state.cursor - increment, 0);
	}

	if (key == VK_UP) editor_state.line = CLAMP_DOWN(editor_state.line - 1, 1);
	if (key == VK_DOWN) editor_state.line++;

	if (editor_state.cursor != cursor && editor_state.cursor < editor_state.buffer_size) {
		//set_cursor_begin(editor_state.cursor);
	}
}
