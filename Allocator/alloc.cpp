#include "alloc.h"

#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <shared_mutex>

std::atomic<size_t> NumAlloc{0};
std::atomic<size_t> NumDealloc{0};
std::atomic<size_t> NumReUse{0};

struct Block {
  size_t size{0};
  void* data{nullptr};
};

std::list<Block*> FreeList{};

void* InitBrk{nullptr};

std::mutex AllocateMutex{};
using Lock = std::unique_lock<std::mutex>;
std::shared_mutex FreeListMutex{};
using ReadLock = std::shared_lock<std::shared_mutex>;
using WriteLock = std::unique_lock<std::shared_mutex>;

int _brk(void* addr) {
  int ret;
  {
    Lock _(AllocateMutex);
    ret = brk(addr);
  }
  if (ret == -1) {
    assert(errno == ENOMEM);
    std::cout << "brk fail, returning nullptr\n";
    exit(EXIT_FAILURE);
  }
  return 0;
}

void* _sbrk(intptr_t increment) {
  void* ptr;
  {
    Lock _(AllocateMutex);
    ptr = sbrk(increment);
  }
  if (ptr == reinterpret_cast<void*>(-1)) {
    assert(errno == ENOMEM);
    std::cout << "sbrk fail, returning nullptr\n";
    exit(EXIT_FAILURE);
  }
  return ptr;
}

Block* AllocateBlock(size_t size) {
  const size_t allocSize = sizeof(Block) + size;
  Block* block = static_cast<Block*>(_sbrk(allocSize));
  block->size = size;
  block->data = reinterpret_cast<char*>(&(block->data)) + sizeof(Block().data);
  return block;
}

Block* GetHeader(void* data) {
  char* dataEnd = reinterpret_cast<char*>(data);
  char* blockBegin = dataEnd - sizeof(Block);
  return reinterpret_cast<Block*>(blockBegin);
}

Block* FirstFit(size_t size) {
  const auto& it =
      std::find_if(std::cbegin(FreeList), std::cend(FreeList),
                   [&size](const auto& block) { return size <= block->size; });
  if (it == std::cend(FreeList)) return nullptr;
  return *it;
}
Block* BestFit(size_t size) {
  bool found = false;
  int64_t minDist = INT64_MAX;
  Block* minBlock = nullptr;
  for (const auto& block : FreeList) {
    int64_t dist = block->size - size;
    if (dist > 0 && dist < minDist) {
      found = true;
      minDist = dist;
      minBlock = block;
    }
  }
  if (!found) return nullptr;
  return minBlock;
}
Block* WorstFit(size_t size) {
  bool found = false;
  int64_t maxDist = 0;
  Block* maxBlock = nullptr;
  for (const auto& block : FreeList) {
    int64_t dist = block->size - size;
    if (dist > 0 && dist > maxDist) {
      found = true;
      maxDist = dist;
      maxBlock = block;
    }
  }
  if (!found) return nullptr;
  return maxBlock;
}

Block* FindBlock(size_t size) {
  return FirstFit(size);
  //   return BestFit(size);
  //   return WorstFit(size);
}

bool BlockExists(Block* block) {
  const auto& it = std::find(std::cbegin(FreeList), std::cend(FreeList), block);
  if (it == std::cend(FreeList)) return false;
  return true;
}

void InitAllocator() {
  InitBrk = _sbrk(0);
  NumAlloc = 0;
  NumDealloc = 0;
  NumReUse = 0;
}

void DestroyAllocator() {
  FreeList.clear();
  _brk(InitBrk);
}

void* Allocate(size_t size) {
  while (true) {
    Block* block;
    {
      ReadLock _r(FreeListMutex);
      block = FindBlock(size);  // check for suitable dealocated blocks to
                                // re-use
    }
    if (block == nullptr) {
      NumAlloc++;
      return AllocateBlock(size)->data;  // allocate new block
    }
    WriteLock _w(FreeListMutex);
    if (BlockExists(block)) {
      FreeList.remove(block);
      NumReUse++;
      return block->data;  // re-use block
    }
  }
}

void DeAllocate(void* data) {
  Block* block = GetHeader(data);
  NumDealloc++;
  WriteLock _w(FreeListMutex);
  FreeList.emplace_back(block);
}
