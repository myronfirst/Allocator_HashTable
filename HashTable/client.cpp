#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

#include "utils.h"

size_t CommandsNum = 100;
int main(int argc, char* argv[]) {
  std::cout << "client process id:  " << ::getpid() << "\n";
  if (argc > 1 && std::stoi(argv[1]) > 0) CommandsNum = std::stoi(argv[1]);
  assert(CommandsNum < FILE_SIZE);

  std::this_thread::sleep_for(std::chrono::milliseconds(
      4 * 1000));  // sleep until shm and semaphores are opened by server
  int fd = ShmOpen(FILE_NAME, O_RDWR);
  FTruncate(fd, FILE_SIZE);
  Buffer* buffer = static_cast<Buffer*>(MMap(fd, FILE_SIZE, PROT_WRITE));
  sem_t* semFull = SemOpen(SEM_FULL, O_RDWR);
  sem_t* semEmpty = SemOpen(SEM_EMPTY, O_RDWR);
  sem_t* semMutex = SemOpen(SEM_MUTEX, O_RDWR);

  std::cout << "Client loop begin\n";
  while (true) {
    Key len = CommandsNum / 3;
    std::vector<Command> cmdVec{};
    for (Key i = 0; i < len; ++i) {
      cmdVec.emplace_back(Command{CommandType::INSERT, i});
    }
    for (Key i = 0; i < len; ++i) {
      cmdVec.emplace_back(Command{CommandType::FIND, i});
    }
    for (Key i = 0; i < len; ++i) {
      cmdVec.emplace_back(Command{CommandType::REMOVE, i});
    }
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(std::begin(cmdVec), std::end(cmdVec), g);  // shuffle

    SemAcquire(semEmpty);
    SemAcquire(semMutex);
    std::cout << "Client acquired semaphores\n";

    buffer->size = cmdVec.size();
    memcpy(buffer->data, cmdVec.data(), cmdVec.size() * sizeof(Command));
    std::cout << "Client has sent " << buffer->size << " commands\n";

    SemRelease(semMutex);
    SemRelease(semFull);
    std::cout << "Client released semaphores\n";

    std::this_thread::sleep_for(std::chrono::milliseconds(4 * 1000));
    std::cout << "Client is done sleeping \n";
  }
  std::cout << "Client loop end\n";

  SemClose(semFull);
  SemClose(semEmpty);
  SemClose(semMutex);
  MUnMap(buffer, FILE_SIZE);
  ShmClose(fd);

  return 0;
}
