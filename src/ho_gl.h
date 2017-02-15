#include <GL/gl.h>

typedef char GLchar;
typedef int GLint;
typedef float GLfloat;
typedef __int64 GLsizeiptr;
typedef int* GLintptr;

#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82

GLuint (__stdcall* glCreateProgram)();
GLuint (__stdcall* glCreateShader)(GLenum shaderType);
void (__stdcall* glShaderSource)(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
void (__stdcall* glCompileShader)(GLuint shader);
void (__stdcall* glGetShaderiv)(GLuint shader, GLenum pname, GLint* params);
void (__stdcall* glGetShaderInfoLog)(GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog);
void (__stdcall* glGetProgramInfoLog)(GLuint program, GLsizei maxLength, GLsizei* length, GLchar* infoLog);
void (__stdcall* glAttachShader)(GLuint program, GLuint shader);
void (__stdcall* glLinkProgram)(GLuint program);
void (__stdcall* glGetProgramiv)(GLuint program, GLenum pname, GLint* params);
void (__stdcall* glValidateProgram)(GLuint program);
void (__stdcall* glUseProgram)(GLuint program);

void (__stdcall* glDeleteProgram)(GLuint program);
void (__stdcall* glDeleteShader)(GLuint shader);
void (__stdcall* glDetachShader)(GLuint program, GLuint shader);

GLint (_stdcall* glGetUniformLocation)(GLuint program, const GLchar* name);
void(_stdcall* glUniform1f)(GLint location, GLfloat v0);
void(_stdcall* glUniform2f)(GLint location, GLfloat v0, GLfloat v1);
void(_stdcall* glUniform3f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
void(_stdcall* glUniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void(_stdcall* glUniform1i)(GLint location, GLint v0);
void(_stdcall* glUniform2i)(GLint location, GLint v0, GLint v1);
void(_stdcall* glUniform3i)(GLint location, GLint v0, GLint v1, GLint v2);
void(_stdcall* glUniform4i)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);

void(_stdcall* glUniform1fv)(GLint location, GLsizei count, const GLfloat* value);
void(_stdcall* glUniform2fv)(GLint location, GLsizei count, const GLfloat* value);
void(_stdcall* glUniform3fv)(GLint location, GLsizei count, const GLfloat* value);
void(_stdcall* glUniform4fv)(GLint location, GLsizei count, const GLfloat* value);
void(_stdcall* glUniform1iv)(GLint location, GLsizei count, const GLint* value);
void(_stdcall* glUniform2iv)(GLint location, GLsizei count, const GLint* value);
void(_stdcall* glUniform3iv)(GLint location, GLsizei count, const GLint* value);
void(_stdcall* glUniform4iv)(GLint location, GLsizei count, const GLint* value);

void(_stdcall* glUniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void(_stdcall* glUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void(_stdcall* glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void(_stdcall* glUniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void(_stdcall* glUniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void(_stdcall* glUniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void(_stdcall* glUniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void(_stdcall* glUniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void(_stdcall* glUniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

void(__stdcall *glGenBuffers)(GLsizei n, GLuint* buffers);
void(__stdcall* glBindBuffer)(GLenum target, GLuint buffer);
void(__stdcall* glBufferData)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
void(__stdcall* glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
void(__stdcall* glDeleteBuffers)(GLsizei n, const GLuint* buffers);
void(__stdcall* glEnableVertexAttribArray)(GLuint index);
void(__stdcall* glDisableVertexAttribArray)(GLuint index);
void(__stdcall* glGetBufferPointerv)(GLenum target, GLenum pname, GLvoid** params);
void*(__stdcall* glMapBuffer)(GLenum target, GLenum access);
GLboolean(__stdcall* glUnmapBuffer)(GLenum target);
void(__stdcall* glDeleteBuffers)(GLsizei n, const GLuint* buffers);
void(__stdcall* glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);

void(__stdcall* glBlendEquation)(GLenum mode);
void(__stdcall* glBlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha);
void(__stdcall* glBlendFuncSeparate)(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);

void(__stdcall* glBindFramebuffer)(GLenum target, GLuint framebuffer);
void(__stdcall* glBindRenderbuffer)(GLenum target, GLuint renderbuffer);
void(__stdcall* glCreateFramebuffers)(GLsizei n, GLuint *ids);
void(__stdcall* glCreateRenderbuffers)(GLsizei n, GLuint *renderbuffers);
void(__stdcall* glDeleteFramebuffers)(GLsizei n, GLuint *framebuffers);
void(__stdcall* glDeleteRenderbuffers)(GLsizei n, GLuint *renderbuffers);
void(__stdcall* glGenFramebuffers)(GLsizei n, GLuint *ids);
void(__stdcall* glGenRenderbuffers)(GLsizei n, GLuint *renderbuffers);

const GLubyte*(__stdcall* glGetStringi)(GLenum name, GLuint index);

void(__stdcall* glBindVertexArray)(GLuint array);
void(__stdcall* glDeleteVertexArrays)(GLsizei n, const GLuint* arrays);
void(__stdcall* glGenVertexArrays)(GLsizei n, GLuint *arrays);

void error_fatal(char* error_type, char* buffer)
{
	HANDLE error_handle = GetStdHandle(STD_ERROR_HANDLE);
	int written = 0;
	WriteConsoleA(error_handle, error_type, strlen(error_type), &written, 0);
	if (buffer) {
		WriteConsoleA(error_handle, buffer, strlen(buffer), &written, 0);
	}
	ExitProcess(-1);
}

void init_gl_extensions()
{
	HMODULE gl_dll = LoadLibraryA("opengl32.dll");
	
	glCreateProgram = (GLuint(__stdcall*)()) wglGetProcAddress("glCreateProgram");
	glCreateShader = (GLuint(__stdcall*)(GLenum)) wglGetProcAddress("glCreateShader");
	glShaderSource = (void(__stdcall*) (GLuint, GLsizei, const GLchar**, const GLint*)) wglGetProcAddress("glShaderSource");
	glCompileShader = (void(__stdcall* )(GLuint)) wglGetProcAddress("glCompileShader");
	glGetShaderiv = (void(__stdcall*)(GLuint, GLenum, GLint*)) wglGetProcAddress("glGetShaderiv");
	glGetShaderInfoLog = (void(__stdcall*)(GLuint, GLsizei, GLsizei*, GLchar*)) wglGetProcAddress("glGetShaderInfoLog");
	glGetProgramInfoLog = (void(__stdcall*)(GLuint, GLsizei, GLsizei*, GLchar*)) wglGetProcAddress("glGetProgramInfoLog");
	glAttachShader = (void(__stdcall*)(GLuint, GLuint)) wglGetProcAddress("glAttachShader");
	glLinkProgram = (void(__stdcall*)(GLuint)) wglGetProcAddress("glLinkProgram");
	glGetProgramiv = (void(__stdcall*)(GLuint, GLenum, GLint*)) wglGetProcAddress("glGetProgramiv");
	glValidateProgram = (void(__stdcall*)(GLuint)) wglGetProcAddress("glValidateProgram");
	glUseProgram = (void(__stdcall*)(GLuint)) wglGetProcAddress("glUseProgram");

	glDeleteProgram = (void(__stdcall*)(GLuint)) wglGetProcAddress("glDeleteProgram");
	glDeleteShader = (void(__stdcall*)(GLuint)) wglGetProcAddress("glDeleteShader");
	glDetachShader = (void(__stdcall*)(GLuint, GLuint)) wglGetProcAddress("glDetachShader");

	glGetUniformLocation = (GLint(_stdcall*)(GLuint program, const GLchar* name)) wglGetProcAddress("glGetUniformLocation");
	glUniform1f = (void(_stdcall*)(GLint, GLfloat)) wglGetProcAddress("glUniform1f");
	glUniform2f = (void(_stdcall*)(GLint, GLfloat, GLfloat)) wglGetProcAddress("glUniform2f");
	glUniform3f = (void(_stdcall*)(GLint, GLfloat, GLfloat, GLfloat)) wglGetProcAddress("glUniform3f");
	glUniform4f = (void(_stdcall*)(GLint, GLfloat, GLfloat, GLfloat, GLfloat)) wglGetProcAddress("glUniform4f");
	glUniform1i = (void(_stdcall*)(GLint, GLint)) wglGetProcAddress("glUniform1i");
	glUniform2i = (void(_stdcall*)(GLint, GLint, GLint)) wglGetProcAddress("glUniform2i");
	glUniform3i = (void(_stdcall*)(GLint, GLint, GLint, GLint)) wglGetProcAddress("glUniform3i");
	glUniform4i = (void(_stdcall*)(GLint, GLint, GLint, GLint, GLint)) wglGetProcAddress("glUniform4i");

	glUniform1fv = (void(_stdcall*)(GLint, GLsizei, const GLfloat*)) wglGetProcAddress("glUniform1fv");
	glUniform2fv = (void(_stdcall*)(GLint, GLsizei, const GLfloat*)) wglGetProcAddress("glUniform2fv");
	glUniform3fv = (void(_stdcall*)(GLint, GLsizei, const GLfloat*)) wglGetProcAddress("glUniform3fv");
	glUniform4fv = (void(_stdcall*)(GLint, GLsizei, const GLfloat*)) wglGetProcAddress("glUniform4fv");
	glUniform1iv = (void(_stdcall*)(GLint, GLsizei, const GLint*)) wglGetProcAddress("glUniform1iv");
	glUniform2iv = (void(_stdcall*)(GLint, GLsizei, const GLint*)) wglGetProcAddress("glUniform2iv");
	glUniform3iv = (void(_stdcall*)(GLint, GLsizei, const GLint*)) wglGetProcAddress("glUniform3iv");
	glUniform4iv = (void(_stdcall*)(GLint, GLsizei, const GLint*)) wglGetProcAddress("glUniform4iv");

	glUniformMatrix2fv = (void(_stdcall*)(GLint, GLsizei, GLboolean, const GLfloat*)) wglGetProcAddress("glUniformMatrix2fv");
	glUniformMatrix3fv = (void(_stdcall*)(GLint, GLsizei, GLboolean, const GLfloat*)) wglGetProcAddress("glUniformMatrix3fv");
	glUniformMatrix4fv = (void(_stdcall*)(GLint, GLsizei, GLboolean, const GLfloat*)) wglGetProcAddress("glUniformMatrix4fv");
	glUniformMatrix2x3fv = (void(_stdcall*)(GLint, GLsizei, GLboolean, const GLfloat*)) wglGetProcAddress("glUniformMatrix2x3fv");
	glUniformMatrix3x2fv = (void(_stdcall*)(GLint, GLsizei, GLboolean, const GLfloat*)) wglGetProcAddress("glUniformMatrix3x2fv");
	glUniformMatrix2x4fv = (void(_stdcall*)(GLint, GLsizei, GLboolean, const GLfloat*)) wglGetProcAddress("glUniformMatrix2x4fv");
	glUniformMatrix4x2fv = (void(_stdcall*)(GLint, GLsizei, GLboolean, const GLfloat*)) wglGetProcAddress("glUniformMatrix4x2fv");
	glUniformMatrix3x4fv = (void(_stdcall*)(GLint, GLsizei, GLboolean, const GLfloat*)) wglGetProcAddress("glUniformMatrix3x4fv");
	glUniformMatrix4x3fv = (void(_stdcall*)(GLint, GLsizei, GLboolean, const GLfloat*)) wglGetProcAddress("glUniformMatrix4x3fv");

	glGenBuffers = (void(__stdcall *)(GLsizei, GLuint*)) wglGetProcAddress("glGenBuffers");
	glBindBuffer = (void(__stdcall*)(GLenum, GLuint)) wglGetProcAddress("glBindBuffer");
	glBufferData = (void(__stdcall*)(GLenum, GLsizeiptr, const GLvoid*, GLenum)) wglGetProcAddress("glBufferData");
	glBufferSubData = (void(__stdcall*)(GLenum, GLintptr, GLsizeiptr, const GLvoid*)) wglGetProcAddress("glBufferSubData");
	glDeleteBuffers = (void(__stdcall*)(GLsizei, const GLuint*)) wglGetProcAddress("glDeleteBuffers");
	glEnableVertexAttribArray = (void(__stdcall*)(GLuint)) wglGetProcAddress("glEnableVertexAttribArray");
	glDisableVertexAttribArray = (void(__stdcall*)(GLuint)) wglGetProcAddress("glDisableVertexAttribArray");
	glGetBufferPointerv = (void(__stdcall*)(GLenum, GLenum, GLvoid**)) wglGetProcAddress("glGetBufferPointerv");
	glMapBuffer = (void*(__stdcall*)(GLenum, GLenum)) wglGetProcAddress("glMapBuffer");
	glUnmapBuffer = (GLboolean(__stdcall*)(GLenum)) wglGetProcAddress("glUnmapBuffer");
	glDeleteBuffers = (void(__stdcall*)(GLsizei, const GLuint*)) wglGetProcAddress("glDeleteBuffers");
	glVertexAttribPointer = (void(__stdcall*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*)) wglGetProcAddress("glVertexAttribPointer");

	glBlendEquation = (void(__stdcall*)(GLenum)) wglGetProcAddress("glBlendEquation");
	glBlendEquationSeparate = (void(__stdcall*)(GLenum, GLenum)) wglGetProcAddress("glBlendEquationSeparate");
	glBlendFuncSeparate = (void(__stdcall*)(GLenum, GLenum, GLenum, GLenum)) wglGetProcAddress("glBlendFuncSeparate");

	glBindFramebuffer = (void(__stdcall*)(GLenum, GLuint)) wglGetProcAddress("glBindFramebuffer");
	glBindRenderbuffer = (void(__stdcall*)(GLenum, GLuint)) wglGetProcAddress("glBindRenderbuffer");
	glCreateFramebuffers = (void(__stdcall*)(GLsizei, GLuint*)) wglGetProcAddress("glCreateFramebuffers");
	glCreateRenderbuffers = (void(__stdcall*)(GLsizei, GLuint*)) wglGetProcAddress("glCreateRenderbuffers");
	glDeleteFramebuffers = (void(__stdcall*)(GLsizei, GLuint*)) wglGetProcAddress("glDeleteFramebuffers");
	glDeleteRenderbuffers = (void(__stdcall*)(GLsizei, GLuint*)) wglGetProcAddress("glDeleteRenderbuffers");
	glGenFramebuffers = (void(__stdcall*)(GLsizei, GLuint*)) wglGetProcAddress("glGenFramebuffers");
	glGenRenderbuffers = (void(__stdcall*)(GLsizei, GLuint*)) wglGetProcAddress("glGenRenderbuffers");

	glGetStringi = (const GLubyte*(__stdcall*)(GLenum, GLuint)) wglGetProcAddress("glGetStringi");

	glBindVertexArray = (void(__stdcall*)(GLuint)) wglGetProcAddress("glBindVertexArray");
	glDeleteVertexArrays = (void(__stdcall*)(GLsizei, const GLuint*)) wglGetProcAddress("glDeleteVertexArrays");
	glGenVertexArrays = (void(__stdcall*)(GLsizei, GLuint *)) wglGetProcAddress("glGenVertexArrays");

	FreeLibrary(gl_dll);
}

void init_opengl(HWND window_handle, HDC* device_context, HGLRC* rendering_context)
{
	int PixelFormat;
	*device_context = GetDC(window_handle);

	PIXELFORMATDESCRIPTOR pfd;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
	pfd.cDepthBits = 32;
	pfd.cColorBits = 24;
	pfd.iPixelType = PFD_TYPE_RGBA;

	PixelFormat = ChoosePixelFormat(*device_context, &pfd);
	if (!SetPixelFormat(*device_context, PixelFormat, &pfd))
		error_fatal("Error creating context pixel descriptor.\n", 0);

	*rendering_context = wglCreateContext(*device_context);
	BOOL error = wglMakeCurrent(*device_context, *rendering_context);
	
	init_gl_extensions();
	
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
}

GLuint load_shader(const char* vert_shader, const char* frag_shader, GLint vert_length, GLint frag_length)
{
	GLuint vs_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);

	GLint compile_status;

	const GLchar* p_v[1] = { vert_shader };
	glShaderSource(vs_id, 1, p_v, &vert_length);

	const GLchar* p_f[1] = { frag_shader };
	glShaderSource(fs_id, 1, p_f, &frag_length);

	glCompileShader(vs_id);
	glGetShaderiv(vs_id, GL_COMPILE_STATUS, &compile_status);
	if (!compile_status) {
		char error_buffer[512] = {0};
		glGetShaderInfoLog(vs_id, sizeof(error_buffer), NULL, error_buffer);
		error_fatal("Error: vertex shader compilation:\n", error_buffer);
	}

	glCompileShader(fs_id);
	glGetShaderiv(fs_id, GL_COMPILE_STATUS, &compile_status);
	if (!compile_status) {
		char error_buffer[512] = {0};
		glGetShaderInfoLog(fs_id, sizeof(error_buffer) - 1, NULL, error_buffer);
		error_fatal("Error: fragment shader compilation:\n", error_buffer);
	}

	GLuint shader_id = glCreateProgram();
	glAttachShader(shader_id, vs_id);
	glAttachShader(shader_id, fs_id);
	glLinkProgram(shader_id);

	glGetProgramiv(shader_id, GL_LINK_STATUS, &compile_status);
	if (compile_status == 0) {
		GLchar error_buffer[512] = {0};
		glGetProgramInfoLog(shader_id, sizeof(error_buffer) - 1, NULL, error_buffer);
		error_fatal("Error: shader link:\n", error_buffer);
	}

	glValidateProgram(shader_id);
	glUseProgram(shader_id);
	return shader_id;
}
