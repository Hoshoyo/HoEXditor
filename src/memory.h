#ifndef HOHEX_MEMORY_H
#define HOHEX_MEMORY_H
#include <windows.h>
#include "common.h"

#include <malloc.h>

typedef struct {
	int id;
	void* to_free_ptr;
} Arena_Base_Pointer;

#define MEGABYTE(N) (N * 1048576)
#define MAX_ARENA 1024
typedef struct {
	s64 block_size;
	int current_using;

	int size[MAX_ARENA];
	int allocated[MAX_ARENA];	// this block is allocated or not
	void* memory[MAX_ARENA];	// address of the memory at the current location

	Arena_Base_Pointer base_ptr[MAX_ARENA];

} Memory_Arena;

void make_arena(s64 block_size);
void* alloc(s64 s, int* id);
void release(int id);

extern Memory_Arena _am;

void* halloc(size_t size);
void hfree(void* block);

#endif	// HOHEX_MEMORY_H
