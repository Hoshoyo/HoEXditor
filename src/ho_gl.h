#ifndef HOHEX_HOGL_H
#define HOHEX_HOGL_H
#include "common.h"
#include <GL/gl.h>
#include "util.h"

typedef char GLchar;
typedef int GLint;
typedef float GLfloat;
typedef __int64 GLsizeiptr;
typedef int* GLintptr;

#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STREAM_DRAW 0x88E0
#define GL_STREAM_READ 0x88E1
#define GL_STREAM_COPY 0x88E2
#define GL_STATIC_DRAW 0x88E4
#define GL_STATIC_READ 0x88E5
#define GL_STATIC_COPY 0x88E6
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_READ 0x88E9
#define GL_DYNAMIC_COPY 0x88EA
#define GL_CLAMP_TO_EDGE 0x812F

#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE4 0x84C4
#define GL_TEXTURE5 0x84C5
#define GL_TEXTURE6 0x84C6
#define GL_TEXTURE7 0x84C7
#define GL_TEXTURE8 0x84C8
#define GL_TEXTURE9 0x84C9
#define GL_TEXTURE10 0x84CA
#define GL_TEXTURE11 0x84CB
#define GL_TEXTURE12 0x84CC
#define GL_TEXTURE13 0x84CD
#define GL_TEXTURE14 0x84CE
#define GL_TEXTURE15 0x84CF
#define GL_TEXTURE16 0x84D0
#define GL_TEXTURE17 0x84D1
#define GL_TEXTURE18 0x84D2
#define GL_TEXTURE19 0x84D3
#define GL_TEXTURE20 0x84D4
#define GL_TEXTURE21 0x84D5
#define GL_TEXTURE22 0x84D6
#define GL_TEXTURE23 0x84D7
#define GL_TEXTURE24 0x84D8
#define GL_TEXTURE25 0x84D9
#define GL_TEXTURE26 0x84DA
#define GL_TEXTURE27 0x84DB
#define GL_TEXTURE28 0x84DC
#define GL_TEXTURE29 0x84DD
#define GL_TEXTURE30 0x84DE
#define GL_TEXTURE31 0x84DF

#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_FLAGS_ARB                   0x2094

bool(__stdcall* wglSwapIntervalEXT)(int interval);
HGLRC (WINAPI* wglCreateContextAttribsARB)(HDC hDC, HGLRC hShareContext, int *attribList);

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
GLint(__stdcall* glGetAttribLocation)(GLuint program, const GLchar* name);


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
void(__stdcall* glActiveTexture)(GLenum texture);

void init_gl_extensions();
void init_opengl(HWND window_handle, HDC* device_context, HGLRC* rendering_context);
GLuint load_shader(const char* vert_shader, const char* frag_shader, GLint vert_length, GLint frag_length);

#ifdef HOGL_IMPLEMENT
void init_gl_extensions()
{
	wglSwapIntervalEXT = (bool(__stdcall*)(int)) wglGetProcAddress("wglSwapIntervalEXT");

	glCreateProgram = (GLuint(__stdcall*)()) wglGetProcAddress("glCreateProgram");
	glCreateShader = (GLuint(__stdcall*)(GLenum)) wglGetProcAddress("glCreateShader");
	glShaderSource = (void(__stdcall*) (GLuint, GLsizei, const GLchar**, const GLint*)) wglGetProcAddress("glShaderSource");
	glCompileShader = (void(__stdcall*)(GLuint)) wglGetProcAddress("glCompileShader");
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
	glGetAttribLocation = (GLint(__stdcall*)(GLuint, const GLchar*)) wglGetProcAddress("glGetAttribLocation");

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
	glActiveTexture = (void(__stdcall*)(GLenum texture)) wglGetProcAddress("glActiveTexture");
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

	HGLRC temp_context = wglCreateContext(*device_context);
	BOOL error = wglMakeCurrent(*device_context, temp_context);

	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 1,
		WGL_CONTEXT_FLAGS_ARB, 0,
		0
	};
	wglCreateContextAttribsARB = (HGLRC(WINAPI*)(HDC, HGLRC, int *))wglGetProcAddress("wglCreateContextAttribsARB");
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(temp_context);
	*rendering_context = wglCreateContextAttribsARB(*device_context, 0, attribs);
	wglMakeCurrent(*device_context, *rendering_context);

	init_gl_extensions();

	glClearColor(30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 1.0f);
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
		char error_buffer[512] = { 0 };
		glGetShaderInfoLog(vs_id, sizeof(error_buffer), NULL, error_buffer);
		error_warning("Error: vertex shader compilation:\n");
		error_warning(error_buffer);
		return -1;
	}

	glCompileShader(fs_id);
	glGetShaderiv(fs_id, GL_COMPILE_STATUS, &compile_status);
	if (!compile_status) {
		char error_buffer[512] = { 0 };
		glGetShaderInfoLog(fs_id, sizeof(error_buffer) - 1, NULL, error_buffer);
		error_warning("Error: fragment shader compilation:\n");
		error_warning(error_buffer);
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
		error_warning("Error: shader link:\n");
		error_warning(error_buffer);
		return -1;
	}

	glValidateProgram(shader_id);
	return shader_id;
}
#endif // HOGL_IMPLEMENT
#endif // HOHEX_HOGL_H