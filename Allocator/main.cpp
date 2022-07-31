#include <atomic>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "alloc.h"

size_t ThreadNum = std::thread::hardware_concurrency();

void* MyMalloc(size_t size) { return Allocate(size); }
void MyFree(void* data) { DeAllocate(data); }

constexpr size_t arrSize = 256;
void ArraysSerial() {
  InitAllocator();

  char* arr0 = (char*)MyMalloc(arrSize * sizeof(char));
  int* arr1 = (int*)MyMalloc(arrSize * sizeof(int));
  double* arr2 = (double*)MyMalloc(arrSize * sizeof(double));

  for (size_t i = 0; i < arrSize; ++i) {
    arr0[i] = i + 'A';
    arr1[i] = i;
    arr2[i] = i + 0.5;
  }
  //   for (size_t i = 0; i < arrSize; ++i) std::cout << arr0[i] << ' ';
  //   std::cout << '\n';
  //   for (size_t i = 0; i < arrSize; ++i) std::cout << arr1[i] << ' ';
  //   std::cout << '\n';
  //   for (size_t i = 0; i < arrSize; ++i) std::cout << arr2[i] << ' ';
  //   std::cout << '\n';

  MyFree(arr0);
  MyFree(arr1);
  MyFree(arr2);

  std::cout << "NumAlloc: " << NumAlloc << "\n";
  std::cout << "NumDealloc: " << NumDealloc << "\n";
  std::cout << "NumReUse: " << NumReUse << "\n";

  DestroyAllocator();
}

void ReUseSerial() {
  InitAllocator();

  int* arr1[arrSize]{nullptr};
  for (size_t i = 0; i < arrSize; ++i) {
    arr1[i] = (int*)MyMalloc(sizeof(int));
    arr1[i][0] = i;
  }
  //   for (size_t i = 0; i < arrSize; ++i) std::cout << arr1[i] << ' ';
  //   std::cout << '\n';
  for (size_t i = 0; i < arrSize; ++i) {
    MyFree(arr1[i]);
  }

  char* arr0[arrSize]{nullptr};
  for (size_t i = 0; i < arrSize; ++i) {
    arr0[i] = (char*)MyMalloc(sizeof(char));
    arr0[i][0] = i + 'A';
  }
  //   for (size_t i = 0; i < arrSize; ++i) std::cout << arr0[i] << ' ';
  //   std::cout << '\n';
  for (size_t i = 0; i < arrSize; ++i) {
    MyFree(arr0[i]);
  }

  std::cout << "NumAlloc: " << NumAlloc << "\n";
  std::cout << "NumDealloc: " << NumDealloc << "\n";
  std::cout << "NumReUse: " << NumReUse << "\n";

  DestroyAllocator();
}

void ThreadJob() {
  // ThreadNum = 16;
  size_t size = 256;
  std::vector<double*> doubleVec{};
  for (size_t i = 0; i < size; ++i) {
    double* d = (double*)MyMalloc(sizeof(double));
    *d = i * 1.0;
    doubleVec.emplace_back(d);
  }
  for (auto d : doubleVec) {
    MyFree(d);
  }
  std::vector<char*> charVec{};
  for (size_t i = 0; i < size; ++i) {
    char* c = (char*)MyMalloc(sizeof(char));
    *c = '0' + i;
    charVec.emplace_back(c);
  }
  for (auto c : charVec) {
    MyFree(c);
  }
}
void ArraysConcurrent() {
  InitAllocator();
  std::vector<std::thread> threadVec{};
  for (size_t i = 0; i < ThreadNum; ++i) {
    threadVec.emplace_back(std::thread([]() { ThreadJob(); }));
  }
  for (auto& t : threadVec) t.join();
  std::cout << "NumAlloc: " << NumAlloc << "\n";
  std::cout << "NumDealloc: " << NumDealloc << "\n";
  std::cout << "NumReUse: " << NumReUse << "\n";
  DestroyAllocator();
}

constexpr size_t NumAllocs = 10 * 1024;
void* SharedArray[128][NumAllocs] = {nullptr};
void MallocJob(size_t index, int size) {
  for (size_t i = 0; i < NumAllocs; ++i) {
    SharedArray[index][i] = MyMalloc(size);
  }
}
void FreeJob(size_t index) {
  for (size_t i = 0; i < NumAllocs; ++i) {
    MyFree(SharedArray[index][i]);
  }
}
void MallocFreeShared() {
  InitAllocator();
  std::vector<std::thread> threadVec{};
  for (size_t i = 0; i < ThreadNum; ++i)
    threadVec.emplace_back(std::thread([i]() { MallocJob(i, 4); }));
  for (auto& t : threadVec) t.join();
  threadVec.clear();

  for (size_t i = 0; i < ThreadNum; ++i)
    threadVec.emplace_back(std::thread([i]() { FreeJob(i); }));
  for (auto& t : threadVec) t.join();
  threadVec.clear();

  for (size_t i = 0; i < ThreadNum; ++i)
    threadVec.emplace_back(std::thread([i]() { MallocJob(i, 1); }));
  for (auto& t : threadVec) t.join();
  threadVec.clear();

  std::cout << "NumAlloc: " << NumAlloc << "\n";
  std::cout << "NumDealloc: " << NumDealloc << "\n";
  std::cout << "NumReUse: " << NumReUse << "\n";
  DestroyAllocator();
}

int main(int argc, char* argv[]) {
  if (argc > 1) ThreadNum = std::stoi(argv[1]);
  //   ArraysSerial();
  //   ReUseSerial();
  //   ArraysConcurrent();
  MallocFreeShared();
  return 0;
}
