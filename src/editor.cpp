internal void editor_update_and_render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	char arr[256] = {};
	string t;
	t.data = arr;
	t.length = 0;
	for (int i = 0; i < 50; ++i) {
		t.length = u32_to_str_base10(i, arr);

		RECT client_rect;
		GetClientRect(app.subwindow, &client_rect);
		s32 width = client_rect.right - client_rect.left;
		s32 height = client_rect.bottom - client_rect.top;
		font_info.window_width = width;
		font_info.window_height = height;

		render_text(&font_info, t, hm::vec2(0, (r32)height - 18.0f * (r32)(i + 1)), hm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		string pipe = MAKE_STRING("|");
		render_text(&font_info, pipe, hm::vec2(10.0f, (r32)height - 18.0f * (r32)(i + 1)), hm::vec4(0.0f, 122.0f / 255.0f, 204.0f / 255.0f, 1.0f));
	}
	font_rendering_flush(&font_info, font_info.shader);

}