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

typedef s32 bool;
#define true 1
#define false 0

#define null 0
#define internal static

#define USE_CRT 1
#define HACKER_THEME 0
#define WHITE_THEME 0

#if USE_CRT
#include <stdio.h>
#include <malloc.h>
#endif

#ifndef _WIN64	// @temporary remove this, only for fixing intellisense
#define _WIN64	// @temporary
#endif			// @temporary

#ifdef _WIN64
#include <windows.h>
#include <windowsx.h>
#else
#error OS not yet supported
#endif

#endif
