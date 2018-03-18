u32 shader_load(string vert_src, string frag_src) {
	GLuint vs_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);

	GLint compile_status;

	const GLchar* p_v[1] = { vert_src.data };
	glShaderSource(vs_id, 1, p_v, &vert_src.length);

	const GLchar* p_f[1] = { frag_src.data };
	glShaderSource(fs_id, 1, p_f, &frag_src.length);

	glCompileShader(vs_id);
	glGetShaderiv(vs_id, GL_COMPILE_STATUS, &compile_status);
	if (!compile_status) {
		char error_buffer[512] = { 0 };
		glGetShaderInfoLog(vs_id, sizeof(error_buffer), NULL, error_buffer);
		printf("%s", error_buffer);
		return -1;
	}

	glCompileShader(fs_id);
	glGetShaderiv(fs_id, GL_COMPILE_STATUS, &compile_status);
	if (!compile_status) {
		char error_buffer[512] = { 0 };
		glGetShaderInfoLog(fs_id, sizeof(error_buffer) - 1, NULL, error_buffer);
		printf("%s", error_buffer);
		return -1;
	}

	GLuint shader_id = glCreateProgram();
	glAttachShader(shader_id, vs_id);
	glAttachShader(shader_id, fs_id);
	glDeleteShader(vs_id);
	glDeleteShader(fs_id);
	glLinkProgram(shader_id);

	glGetProgramiv(shader_id, GL_LINK_STATUS, &compile_status);
	if (compile_status == 0) {
		GLchar error_buffer[512] = { 0 };
		glGetProgramInfoLog(shader_id, sizeof(error_buffer) - 1, NULL, error_buffer);
		printf("%s", error_buffer);
		return -1;
	}

	glValidateProgram(shader_id);
	return shader_id;
}

u32 vbo, vao, ebo;

typedef struct {
	hm::vec3 position;
	hm::vec2 texcoord;
	hm::vec4 color;
} Vertex3D;

typedef struct {
	r32 left;
	r32 right;
	r32 top;
	r32 bottom;
} RectBase;

u32 immediate_quad_shader;

string immquad_vshader = MAKE_STRING(R"(
	#version 330 core
	layout(location = 0) in vec3 vertex;
	layout(location = 1) in vec2 tcoords;
	layout(location = 2) in vec4 v_color;

	out vec2 texcoords;
	out vec4 out_color;

	uniform mat4 projection = mat4(1.0);

	void main(){
		gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
		texcoords = tcoords;
		out_color = v_color;
	}
)");

string immquad_fshader = MAKE_STRING(R"(
	#version 330 core
	in vec2 texcoords;
	in vec4 out_color;
	out vec4 color;

	void main(){
		color = out_color;
	}
)");

void init_immediate_quad_mode() {
	immediate_quad_shader = shader_load(immquad_vshader, immquad_fshader);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	const int num_vertices = 4;
	const int num_attribs = 9;

	u16 indices[] = { 0, 1, 2, 2, 3, 0 };
	float s = 100.5f;
	Vertex3D data[4] = {
		{ hm::vec3(-s, -s, 0), hm::vec2(0, 0), hm::vec4(1, 0, 0, 1) },
		{ hm::vec3(s, -s, 0), hm::vec2(1, 0), hm::vec4(0, 1, 0, 1) },
		{ hm::vec3(s,  s, 0), hm::vec2(1, 1), hm::vec4(0, 0, 1, 1) },
		{ hm::vec3(-s,  s, 0), hm::vec2(0, 1), hm::vec4(1, 1, 0, 1) }
	};

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(Vertex3D), data, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)&((Vertex3D*)0)->position);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)&((Vertex3D*)0)->texcoord);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)&((Vertex3D*)0)->color);

	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
}

void immediate_quad(r32 l, r32 r, r32 t, r32 b, hm::vec4 color) {
	Vertex3D data[4] = {
		{ hm::vec3(l, b, 0),  hm::vec2(0, 0), color },
		{ hm::vec3(r, b, 0),  hm::vec2(1, 0), color },
		{ hm::vec3(r,  t, 0), hm::vec2(1, 1), color },
		{ hm::vec3(l,  t, 0), hm::vec2(0, 1), color }
	};
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	void* buffer_dst = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	if (!buffer_dst)
		return;

	memcpy(buffer_dst, data, sizeof(data));

	glUnmapBuffer(GL_ARRAY_BUFFER);

	glUseProgram(immediate_quad_shader);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	hm::mat4 projection = hm::mat4::ortho(0, win_state.win_width, 0, win_state.win_height);
	GLuint loc = glGetUniformLocation(immediate_quad_shader, "projection");
	glUniformMatrix4fv(loc, 1, GL_TRUE, projection.data);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
}

void immediate_rect_border(RectBase* base, r32 thickness, r32 l, r32 r, r32 t, r32 b, hm::vec4 color) {
	r32 baseleft   = base->left + l;
	r32 basebottom = base->bottom + b;
	r += baseleft;
	t += basebottom;
	
	immediate_quad(baseleft, baseleft + thickness, t, basebottom, color);	// left
	immediate_quad(r - thickness, r, t, basebottom, color); // right
	immediate_quad(baseleft, r, t, t - thickness, color); // top
	immediate_quad(baseleft, r, basebottom + thickness, basebottom, color); // bot
}