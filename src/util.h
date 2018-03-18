#ifndef HOHEX_UTIL_H
#define HOHEX_UTIL_H

#include "common.h"

#ifdef USE_CRT
#define assert(EXP) if(!(EXP)) { printf("Assertion failed at %s (%d)", __FILE__, __LINE__); DebugBreak(); }
#endif

#define FLAG(N) (1 << N)
#define ARRAY_COUNT(X) (sizeof(X) / sizeof(*X))

#define FOR(N) for(int i = 0; i < N; ++i)
#define MAX(L, R) ((L > R) ? L : R)
#define MIN(L, R) ((L < R) ? L : R)

#define CLAMP_DOWN(V, MIN) ((V < MIN) ? MIN : V)
#define CLAMP_UP(V, MAX) ((V > MAX) ? MAX : V)
#define CLAMP(V, MIN, MAX) ((V < MIN) ? MIN : (V > MAX) ? MAX : V)

#define NUM_TO_LOWNIBBLE(X) (((X) & 0x0f) >= 0xa) ? ((X) & 0x0f) + 0x37 : ((X) & 0x0f) + 0x30
#define NUM_TO_HIGNIBBLE(X) ((((X) & 0xf0) >> 4) >= 0xa) ? (((X) & 0xf0) >> 4) + 0x37 : (((X) & 0xf0) >> 4) + 0x30

#define MINS32 -2147483648
#define MAXS32 2147483647
#define MAXS64 9223372036854775807
#define MINS64 -9223372036854775808
#define MAXU64 18446744073709551615

u8* read_entire_file(wchar_t* filename, s64* out_size);
bool does_path_exist(wchar_t* path);

void  error_fatal(char* error_type, char* buffer);
void  error_warning(char* error);
void  log_success(char* msg);
void  print(char* msg, ...);
int   print_once(char* msg, ...);
void* memory_alloc(size_t size);
void  memory_free(void* block);

u32 hstrlen(char* str);
extern "C" void memory_copy(u8* dest, u8* src, u32 size);
bool memory_equal(u8* str1, u8* str2, s64 size);
bool string_equal(u8* str1, u8* str2);

// misc
int u32_to_str_base10(u32 val, char* buffer);
int u64_to_str_base10(u64 val, char* buffer);
int s32_to_str_base10(s32 val, char* buffer);
int s64_to_str_base10(s64 val, char* buffer);
int u64_to_str_base16(u64 val, bool leading_zeros, char* buffer);
int u8_to_str_base16(u8 val, bool leading_zeros, char* buffer);

void init_timer();
r64 get_time();

typedef struct {
	s8* data;
	s32 length;
} string;

#define MAKE_STRING(L) { L, sizeof(L) }

#endif // HOHEX_UTIL_H
