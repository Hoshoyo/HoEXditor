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

void log_msg(char* msg) {
	HANDLE cout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	int written = 0;
	WriteConsoleA(cout_handle, msg, strlen(msg), &written, 0);
}
void log_msg_size(char* msg, s32 size)
{
	HANDLE cout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	int written = 0;
	WriteConsoleA(cout_handle, msg, size, &written, 0);
}

s32 buffer_print(char* dst, s32 max, char* src) {
	s32 n = 0;
	char* at = dst;

	while (*at != 0 && n < max) {
		*at++ = *src++;
		n++;
	}
	return n;
}

int u32_to_str_base10(u32 val, char* buffer)
{
	u32 aux = val;
	u32 divisor = 1;
	char numbuffer[32] = { 0 };
	char* at = &numbuffer[31];

	int count = 0;
	u32 accumulated = aux;
	while (accumulated) {
		u32 v = (aux / divisor) % 10;
		accumulated -= v * divisor;
		*at-- = v + 0x30;
		divisor *= 10;
		count++;
	}
	for (int i = 0; i < count; ++i) {
		*buffer++ = *(++at);
	}
	if (val < 0) count++;
	return count + 1;
}

int s32_to_str_base10(s32 val, char* buffer)
{
	s32 aux = (val < 0) ? (u64)-val : (u64)val;
	s32 divisor = 1;
	if (val < 0) *buffer++ = '-';

	char numbuffer[32] = { 0 };
	char* at = &numbuffer[31];

	int count = 0;
	s32 accumulated = aux;
	while (accumulated) {
		s32 v = (aux / divisor) % 10;
		accumulated -= v * divisor;
		*at-- = v + 0x30;
		divisor *= 10;
		count++;
	}
	for (int i = 0; i < count; ++i) {
		*buffer++ = *(++at);
	}
	if (val < 0) count++;
	return count + 1;
}

int u64_to_str_base10(u64 val, char* buffer)
{
	u64 aux = val;
	u64 divisor = 1;
	char numbuffer[64] = { 0 };
	char* at = &numbuffer[63];

	int count = 0;
	u64 accumulated = aux;
	while (accumulated) {
		u64 v = (aux / divisor) % 10;
		accumulated -= v * divisor;
		*at-- = v + 0x30;
		divisor *= 10;
		count++;
	}
	for (int i = 0; i < count; ++i) {
		*buffer++ = *(++at);
	}
	if (val < 0) count++;
	return count + 1;
}

int s64_to_str_base10(s64 val, char* buffer)
{
	s64 aux = (val < 0) ? -val : val;
	s64 divisor = 1;
	if (val < 0) *buffer++ = '-';

	char numbuffer[64] = { 0 };
	char* at = &numbuffer[63];

	int count = 0;
	s64 accumulated = aux;
	while (accumulated) {
		s64 v = (aux / divisor) % 10;
		accumulated -= v * divisor;
		*at-- = v + 0x30;
		divisor *= 10;
		count++;
	}
	for (int i = 0; i < count; ++i) {
		*buffer++ = *(++at);
	}
	if (val < 0) count++;
	return count + 1;
}

int u64_to_str_base16(u64 val, bool leading_zeros, char* buffer)
{
	char numbuffer[64] = { 0 };
	char* at = &numbuffer[63];

	int count = 0;
	u64 mask = 0x0000000f;
	u64 auxmask = 0xffffffffffffffff;
	while (count < 16) {
		if (!(auxmask & val) && !leading_zeros) break;
		u32 v = (val & (mask << (count * 4))) >> (count * 4);
		if (v >= 0x0A) v += 0x37;
		else v += 0x30;
		*at-- = v;
		auxmask &= ~mask << (count * 4);
		count++;
	}
	for (int i = 0; i < count; ++i) {
		*buffer++ = *(++at);
	}
	return count + 1;
}

int u32_to_str_base16(u32 val, bool leading_zeros, char* buffer)
{
	char numbuffer[64] = { 0 };
	char* at = &numbuffer[63];

	int count = 0;
	u32 mask = 0x0000000f;
	u32 auxmask = 0xffffffff;
	while (count < 8) {
		if (!(auxmask & val) && !leading_zeros) break;
		u32 v = (val & (mask << (count * 4))) >> (count * 4);
		if (v >= 0x0A) v += 0x37;
		else v += 0x30;
		*at-- = v;
		auxmask &= ~mask << (count * 4);
		count++;
	}
	for (int i = 0; i < count; ++i) {
		*buffer++ = *(++at);
	}
	return count + 1;
}

void flush_buffer(char* buffer, char** ptr) {
	if (buffer == *ptr) return;
	log_msg_size(buffer, *ptr - buffer);
	*ptr = buffer;
}

void print(char* msg, ...)
{
#define BUFFER_SIZE 1024
	u8 buffer[BUFFER_SIZE];
	u8* at = msg;
	u8* bufptr = buffer;
	s32 count = 0;

	va_list args;
	va_start(args, msg);

	while (*at != '\0') {
		if (at[0] == '%') {
			int advance = 0;
			if (at[1] == 'd') {
				flush_buffer(buffer, &bufptr);
				advance = s32_to_str_base10(va_arg(args, s32), buffer);
				log_msg_size(buffer, advance - 1);
				at++;
			} else if (at[1] == 'x') {
				flush_buffer(buffer, &bufptr);
				advance = u32_to_str_base16(va_arg(args, u32), true, buffer);
				log_msg_size(buffer, advance - 1);
				at++;
			} else if (at[1] == 'p') {
				flush_buffer(buffer, &bufptr);
				advance = u64_to_str_base16(va_arg(args, u64), true, buffer);
				log_msg_size(buffer, advance - 1);
				at++;
			} else if (at[1] == 'q') {
				flush_buffer(buffer, &bufptr);
				advance = s64_to_str_base10(va_arg(args, s64), buffer);
				log_msg_size(buffer, advance - 1);
				at++;
			} else if (at[1] == 'u'){
				flush_buffer(buffer, &bufptr);
				if (at[2] == 'q') {
					advance = u64_to_str_base10(va_arg(args, u64), buffer);
					at++;
				} else {
					advance = u32_to_str_base10(va_arg(args, u32), buffer);
				}
				log_msg_size(buffer, advance - 1);
				at++;
			} else if (at[1] == 's') {
				flush_buffer(buffer, &bufptr);
				char* str = va_arg(args, char*);
				do {
					advance = buffer_print(buffer, BUFFER_SIZE, str);
					str += advance;
					log_msg_size(buffer, advance - 1);
				} while (advance == BUFFER_SIZE);
				at++;
			} else if (at[1] == 'c') {
				flush_buffer(buffer, &bufptr);
				char c = va_arg(args, char);
				log_msg_size(&c, 1);
				at++;
			} else if (at[1] == '.' && at[2] == '*' && at[3] == 's') {
				flush_buffer(buffer, &bufptr);
				s32 length = va_arg(args, s32);
				char* str = va_arg(args, char*);
				do {
					advance = buffer_print(buffer, MIN(length, BUFFER_SIZE), str);
					str += advance;
					log_msg_size(buffer, MIN(length, BUFFER_SIZE));
					length -= advance;
				} while (advance == BUFFER_SIZE);
				at += 3;
			}
			at++;
		}
		else {
			*bufptr = *at;
			at++;
			bufptr++;
			count++;
		}
		if (count == BUFFER_SIZE || *at == 0) {
			flush_buffer(buffer, &bufptr);
		}
	}
	va_end(args);
#undef BUFFER_SIZE
}
