#include "utils.h"

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>

const char* FILE_NAME = "/shared_file";
const char* SEM_FULL = "/semaphore_full";
const char* SEM_EMPTY = "/semaphore_empty";
const char* SEM_MUTEX = "/semaphore_mutex";

int ShmOpen(const char* name, int oflag) {
  int ret = shm_open(name, oflag, 0666);
  if (ret == -1) {
    std::cout << "File:" << __FILE__ << " Line: " << __LINE__
              << " errno: " << errno;
    std::cout << " shm_open failure\n";
    exit(EXIT_FAILURE);
  }
  return ret;
}
int ShmClose(int fd) {
  int ret = close(fd);
  if (ret != 0) {
    std::cout << "File:" << __FILE__ << " Line: " << __LINE__
              << " errno: " << errno;
    std::cout << " close failure\n";
    exit(EXIT_FAILURE);
  }
  return ret;
}
int ShmUnlink(const char* name) {
  int ret = shm_unlink(name);
  if (ret == -1) {
    std::cout << "File:" << __FILE__ << " Line: " << __LINE__
              << " errno: " << errno;
    std::cout << " shm_unlink failure\n";
    exit(EXIT_FAILURE);
  }
  return ret;
}
int FTruncate(int fd, off_t length) {
  int ret = ftruncate(fd, length);
  if (ret != 0) {
    std::cout << "File:" << __FILE__ << " Line: " << __LINE__
              << " errno: " << errno;
    std::cout << " ftruncate failure\n";
    exit(EXIT_FAILURE);
  }
  return ret;
}
void* MMap(int fd, size_t length, int prot) {
  void* ret = mmap(0, length, prot, MAP_SHARED, fd, 0);
  if (ret == MAP_FAILED) {
    std::cout << "File:" << __FILE__ << " Line: " << __LINE__
              << " errno: " << errno;
    std::cout << " mmap failure\n";
    exit(EXIT_FAILURE);
  }
  return ret;
}
int MUnMap(void* addr, size_t length) {
  int ret = munmap(addr, length);
  if (ret != 0) {
    std::cout << "File:" << __FILE__ << " Line: " << __LINE__
              << " errno: " << errno;
    std::cout << " munmap failure\n";
    exit(EXIT_FAILURE);
  }
  return ret;
}

sem_t* SemOpen(const char* name, int oflag) {
  sem_t* ret = sem_open(name, oflag);
  if (ret == SEM_FAILED) {
    std::cout << "File:" << __FILE__ << " Line: " << __LINE__
              << " errno: " << errno;
    std::cout << " sem_open failure\n";
    exit(EXIT_FAILURE);
  }
  return ret;
}
sem_t* SemOpen(const char* name, int oflag, int value) {
  sem_t* ret = sem_open(name, oflag, 0666, value);
  if (ret == SEM_FAILED) {
    std::cout << "File:" << __FILE__ << " Line: " << __LINE__
              << " errno: " << errno;
    std::cout << " sem_open failure\n";
    exit(EXIT_FAILURE);
  }
  return ret;
}
int SemClose(sem_t* sem) {
  int ret = sem_close(sem);
  if (ret != 0) {
    std::cout << "File:" << __FILE__ << " Line: " << __LINE__
              << " errno: " << errno;
    std::cout << " sem_close failure\n";
    exit(EXIT_FAILURE);
  }
  return ret;
}
int SemUnlink(const char* name) {
  int ret = sem_unlink(name);
  if (ret != 0) {
    std::cout << "File:" << __FILE__ << " Line: " << __LINE__
              << " errno: " << errno;
    std::cout << " sem_unlink failure\n";
    exit(EXIT_FAILURE);
  }
  return ret;
}
int SemAcquire(sem_t* sem) {
  int ret = sem_wait(sem);
  if (ret != 0) {
    std::cout << "File:" << __FILE__ << " Line: " << __LINE__
              << " errno: " << errno;
    std::cout << " sem_wait failure\n";
    exit(EXIT_FAILURE);
  }
  return ret;
}
int SemRelease(sem_t* sem) {
  int ret = sem_post(sem);
  if (ret != 0) {
    std::cout << "File:" << __FILE__ << " Line: " << __LINE__
              << " errno: " << errno;
    std::cout << " sem_post failure\n";
    exit(EXIT_FAILURE);
  }
  return ret;
}
