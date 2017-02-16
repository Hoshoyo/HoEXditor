#include "util.h"
#include "memory.h"
#include <windows.h>

u8* read_entire_file(u8* filename, s64* out_size)
{
	/* get file size */
	WIN32_FIND_DATA info;
	HANDLE search_handle = FindFirstFileEx(filename, FindExInfoStandard, &info, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
	if (search_handle == INVALID_HANDLE_VALUE) return 0;
	FindClose(search_handle);
	u64 file_size = (u64)info.nFileSizeLow | ((u64)info.nFileSizeHigh << 32);
	if(out_size) *out_size = file_size;
	/* open file */
	HANDLE fhandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	int error = GetLastError();

	u64 maxs32 = 0x7fffffff;
	DWORD bytes_read = 0;
	if (file_size == 0) return 0;				// error file is empty
	void* memory = halloc(file_size);
	if (INVALID_HANDLE_VALUE == fhandle) return 0;

	if (file_size > maxs32) {
		void* mem_aux = memory;
		s64 num_reads = file_size / maxs32;
		s64 rest = file_size % maxs32;
		DWORD bytes_read;
		for (s64 i = 0; i < num_reads; ++i) {
			ReadFile(fhandle, mem_aux, (DWORD)maxs32, &bytes_read, 0);
			mem_aux = (char*)mem_aux + maxs32;
		}
		ReadFile(fhandle, mem_aux, (DWORD)rest, &bytes_read, 0);
	} else {
		ReadFile(fhandle, memory, (DWORD)file_size, &bytes_read, 0);
	}
	CloseHandle(fhandle);
	return memory;
}

#define LOG_LEVEL_FATAL 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_SUCCESS 3

s16 set_error_color(u32 level, HANDLE hConsole)
{
	CONSOLE_SCREEN_BUFFER_INFO console_info;
	GetConsoleScreenBufferInfo(hConsole, &console_info);

	switch (level) {
	case LOG_LEVEL_SUCCESS:
		SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		break;
	case LOG_LEVEL_FATAL:
		SetConsoleTextAttribute(hConsole, BACKGROUND_RED | BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		break;
	case LOG_LEVEL_ERROR:
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
		break;
	case LOG_LEVEL_WARN:
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		break;
	}
	return console_info.wAttributes;
}

void reset_error_color(s16 attribute, HANDLE hConsole)
{
	SetConsoleTextAttribute(hConsole, attribute);
}

void error_fatal(char* error_type, char* buffer)
{
	HANDLE error_handle = GetStdHandle(STD_ERROR_HANDLE);
	int written = 0;
	s64 att = set_error_color(LOG_LEVEL_FATAL, error_handle);
	WriteConsoleA(error_handle, error_type, strlen(error_type), &written, 0);
	if (buffer) {
		WriteConsoleA(error_handle, buffer, strlen(buffer), &written, 0);
	}
	reset_error_color(att, error_handle);
	MessageBoxA(0, "A fatal error occurred, check log.\n", "Fatal Error", 0);
	ExitProcess(-1);
}

void error_warning(char* error)
{
	HANDLE error_handle = GetStdHandle(STD_ERROR_HANDLE);
	int written = 0;
	s64 att = set_error_color(LOG_LEVEL_WARN, error_handle);
	WriteConsoleA(error_handle, error, strlen(error), &written, 0);
	reset_error_color(att, error_handle);
}

void log_success(char* msg)
{
	HANDLE cout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	int written = 0;
	s64 att = set_error_color(LOG_LEVEL_SUCCESS, cout_handle);
	WriteConsoleA(cout_handle, msg, strlen(msg), &written, 0);
	reset_error_color(att, cout_handle);
}