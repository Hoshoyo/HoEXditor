#ifndef HOHEX_COMMON_H
#define HOHEX_COMMON_H

typedef int 		s32;
typedef long long 	s64;
typedef short 		s16;
typedef char		s8;
typedef unsigned int 		u32;
typedef unsigned long long 	u64;
typedef unsigned short 		u16;
typedef unsigned char		u8;

typedef float r32;
typedef double r64;

#ifndef __cplusplus
typedef s32 bool;
#define true 1
#define false 0
#endif

#define null 0
#define internal static
#define global_variable static

#define USE_CRT 1
#define HACKER_THEME 0
#define WHITE_THEME 1

#if USE_CRT
#include <stdio.h>
#include <malloc.h>
#endif

#ifdef _WIN64
#define UNICODE
#include <windows.h>
#include <windowsx.h>
#else
#error OS not yet supported
#endif

#endif
