#ifndef _ALLOC_H_
#define _ALLOC_H_

#include <atomic>
#include <cstddef>

extern std::atomic<size_t> NumAlloc;
extern std::atomic<size_t> NumDealloc;
extern std::atomic<size_t> NumReUse;

void InitAllocator();
void DestroyAllocator();
void* Allocate(size_t size);
void DeAllocate(void* data);

#endif
