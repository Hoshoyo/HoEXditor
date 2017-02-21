#ifndef HOHEX_UTIL_H
#define HOHEX_UTIL_H

#include "common.h"

#define FLAG(N) (1 << N)

#define FOR(N) for(int i = 0; i < N; ++i)
#define MAX(L, R) ((L > R) ? L : R)
#define MIN(L, R) ((L < R) ? L : R)

#define MINS32 -2147483648
#define MAXS32 2147483647
#define MAXS64 9223372036854775807
#define MINS64 -9223372036854775808
#define MAXU64 18446744073709551615

u8* read_entire_file(u8* filename, s64* out_size);
void error_fatal(char* error_type, char* buffer);
void error_warning(char* error);
void log_success(char* msg);
void print(char* msg, ...);

u32 hstrlen(char* str);
void copy_string(u8* dest, u8* src, u32 size);

#define CLAMP_DOWN(V, MIN) ((V < MIN) ? MIN : V)
#define CLAMP_UP(V, MAX) ((V > MAX) ? MAX : V)
#define CLAMP(V, MIN, MAX) ((V < MIN) ? MIN : (V > MAX) ? MAX : V)

// misc

int u32_to_str_base10(u32 val, char* buffer);
int u64_to_str_base10(u64 val, char* buffer);
int s32_to_str_base10(s32 val, char* buffer);
int s64_to_str_base10(s64 val, char* buffer);
int u64_to_str_base16(u64 val, bool leading_zeros, char* buffer);
int u8_to_str_base16(u8 val, bool leading_zeros, char* buffer);

#endif // HOHEX_UTIL_H
