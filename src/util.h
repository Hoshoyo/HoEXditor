#ifndef HOHEX_UTIL_H
#define HOHEX_UTIL_H

#include "common.h"

#define FOR(N) for(int i = 0; i < N; ++i)

u8* read_entire_file(u8* filename, s64* out_size);
void error_fatal(char* error_type, char* buffer);
void error_warning(char* error);
void log_success(char* msg);

#endif // HOHEX_UTIL_H