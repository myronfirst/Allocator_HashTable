#ifndef _UTILS_H_
#define _UTILS_H_

#include <semaphore.h>

extern const char* FILE_NAME;
extern const char* SEM_FULL;
extern const char* SEM_EMPTY;
extern const char* SEM_MUTEX;

using Key = int64_t;
enum class CommandType { FIND = 0, INSERT = 1, REMOVE = 2 };

struct Command {
  CommandType type{};
  Key key{};
};

constexpr size_t FILE_SIZE = 32 * 1024 * sizeof(Command);

struct Buffer {
  int size;
  Command data[FILE_SIZE - 1024];
};

int ShmOpen(const char* name, int oflag);
int ShmClose(int fd);
int ShmUnlink(const char* name);
int FTruncate(int fd, off_t length);
void* MMap(int fd, size_t length, int prot);
int MUnMap(void* addr, size_t length);

sem_t* SemOpen(const char* name, int oflag);
sem_t* SemOpen(const char* name, int oflag, int value);
int SemClose(sem_t* sem);
int SemUnlink(const char* name);
int SemAcquire(sem_t* sem);
int SemRelease(sem_t* sem);

#endif
