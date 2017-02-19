#ifndef HOHEX_UTIL_H
#define HOHEX_UTIL_H

#include "common.h"

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

#endif // HOHEX_UTIL_H
