#include "memory.h"
Memory_Arena _am;

void make_arena(s64 block_size)
{
	_am.block_size = block_size;
	_am.current_using = 0;
	ZeroMemory(_am.size, sizeof(int) * MAX_ARENA);
	ZeroMemory(_am.allocated, sizeof(int) * MAX_ARENA);
	_am.memory[0] = VirtualAlloc(0, _am.block_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	_am.base_ptr[0].to_free_ptr = _am.memory[0];
	_am.allocated[0] = 1;
}

int arena_first_free()
{
	for (int i = 0; i < MAX_ARENA; ++i) {
		if (_am.allocated[i] == 0) return i;
	}
	return -1;
}

void* alloc(s64 s, int* id)
{
	if (s > _am.block_size) return 0;	// error, allocation bigger than block size of arena
	if (_am.size[_am.current_using] + s > _am.block_size || _am.allocated[_am.current_using] == 0) {
		_am.current_using = arena_first_free();
		if (_am.current_using == -1) return 0;			// error, full arena
		_am.size[_am.current_using] = s;
		_am.memory[_am.current_using] = VirtualAlloc(0, _am.block_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		_am.allocated[_am.current_using] = 1;
		_am.base_ptr[_am.current_using].to_free_ptr = _am.memory[_am.current_using];
		_am.base_ptr[_am.current_using].id = _am.current_using;
		*id = _am.base_ptr[_am.current_using].id;
		return _am.memory[_am.current_using];
	}
	void* result = _am.memory[_am.current_using];
	_am.memory[_am.current_using] = (char*)_am.memory[_am.current_using] + s;
	_am.size[_am.current_using] += s;
	*id = _am.current_using;
	return result;
}
void release(int id)
{
	_am.allocated[id] = 0;
	_am.size[id] = 0;
	VirtualFree(_am.base_ptr[id].to_free_ptr, _am.block_size, MEM_RELEASE);
}

void* halloc(size_t size) {
	return malloc(size);
}
void hfree(void* block) {
	free(block);
}