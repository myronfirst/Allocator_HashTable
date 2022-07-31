#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <thread>

#include "table.h"

size_t ThreadNum = std::thread::hardware_concurrency();
Table HashTable{};
std::vector<Command> CmdVec{};

void ThreadJob(const size_t& id) {
  const auto& step = CmdVec.size() / ThreadNum;
  const auto& begin = std::cbegin(CmdVec) + (id * step);
  auto end = begin + step;
  if (end > std::cend(CmdVec)) end = std::cend(CmdVec);
  std::cout << "Thread id " << id << " has been assigned "
            << std::distance(begin, end) << " commands\n";
  std::vector<Command> cmdVecThread{begin, end};
  for (const auto& cmd : cmdVecThread) {
    switch (cmd.type) {
      case CommandType::FIND: {
        HashTable.Find(cmd.key);
        break;
      }
      case CommandType::INSERT: {
        HashTable.Insert(cmd.key);
        break;
      }
      case CommandType::REMOVE: {
        HashTable.Remove(cmd.key);
        break;
      }
      default: {
        assert(false);
        break;
      }
    }
  }
}

/*
len successfull inserts
len successfull finds
len successfull removes
len failed inserts
len failed finds
len failed removes
*/
void TestSerial() {
  Key len = 1 * 100000;
  std::vector<Command> cmdVec{};
  for (Key i = 0; i < len; ++i)
    cmdVec.emplace_back(Command{CommandType::INSERT, i});  // success
  for (Key i = 0; i < len; ++i)
    cmdVec.emplace_back(Command{CommandType::INSERT, i});  // fail

  for (Key i = 0; i < len; ++i)
    cmdVec.emplace_back(Command{CommandType::FIND, i});  // success
  for (Key i = 0; i < len; ++i)
    cmdVec.emplace_back(Command{CommandType::REMOVE, i});  // success

  for (Key i = len; i < len + len; ++i)
    cmdVec.emplace_back(Command{CommandType::FIND, i});  // fail
  for (Key i = len; i < len + len; ++i)
    cmdVec.emplace_back(Command{CommandType::REMOVE, i});  // fail
  for (const auto& cmd : cmdVec) {
    switch (cmd.type) {
      case CommandType::FIND: {
        HashTable.Find(cmd.key);
        break;
      }
      case CommandType::INSERT: {
        HashTable.Insert(cmd.key);
        break;
      }
      case CommandType::REMOVE: {
        HashTable.Remove(cmd.key);
        break;
      }
      default: {
        assert(false);
        break;
      }
    }
  }
  std::cout << HashTable.GetStats(false) << "\n";
  CmdVec.clear();
  //   HashTable.Clear();
}

/*
0 - len inserts
0 - len inserts, same as above
0 - len finds
0 - len removes
len - 2*len finds, all fail
len - 2*len removes, all fail
commands can be shuffled
*/
void TestConcurrent() {
  ThreadNum = 8;
  Key len = 6 * 1000000;
  for (Key i = 0; i < len; ++i)
    CmdVec.emplace_back(Command{CommandType::INSERT, i});
  for (Key i = 0; i < len; ++i)
    CmdVec.emplace_back(Command{CommandType::INSERT, i});

  for (Key i = 0; i < len; ++i)
    CmdVec.emplace_back(Command{CommandType::FIND, i});
  for (Key i = 0; i < len; ++i)
    CmdVec.emplace_back(Command{CommandType::REMOVE, i});

  for (Key i = len; i < len + len; ++i)
    CmdVec.emplace_back(Command{CommandType::FIND, i});
  for (Key i = len; i < len + len; ++i)
    CmdVec.emplace_back(Command{CommandType::REMOVE, i});

  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(std::begin(CmdVec), std::end(CmdVec), g);  // shuffle

  std::vector<std::thread> threadVec{};
  for (size_t i = 0; i < ThreadNum; ++i)
    threadVec.emplace_back(std::thread([i]() { ThreadJob(i); }));
  for (auto& t : threadVec) t.join();

  std::cout << HashTable.GetStats(false) << "\n";
  CmdVec.clear();
  //   HashTable.Clear();
}

int main(int argc, char* argv[]) {
  std::cout << "server process id:  " << ::getpid() << "\n";
  if (argc > 1 && std::stoi(argv[1]) > 0) ThreadNum = std::stoi(argv[1]);

  //   TestSerial();
  //   TestConcurrent();
  //   return 0;

  int fd = ShmOpen(FILE_NAME, O_RDONLY | O_CREAT | O_TRUNC);
  Buffer* buffer = static_cast<Buffer*>(MMap(fd, FILE_SIZE, PROT_READ));
  sem_t* semFull = SemOpen(SEM_FULL, O_RDWR | O_CREAT | O_TRUNC, 0);
  sem_t* semEmpty = SemOpen(SEM_EMPTY, O_RDWR | O_CREAT | O_TRUNC, 1);
  sem_t* semMutex = SemOpen(SEM_MUTEX, O_RDWR | O_CREAT | O_TRUNC, 1);

  std::cout << "Server loop begin\n";
  while (true) {
    SemAcquire(semFull);
    SemAcquire(semMutex);
    std::cout << "Server acquired semaphores\n";

    CmdVec.assign(&buffer->data[0], &buffer->data[buffer->size]);
    std::cout << "Server has received " << buffer->size << " commands\n";

    SemRelease(semMutex);
    SemRelease(semEmpty);
    std::cout << "Server released semaphores\n";

    std::vector<std::thread> serverThreads{};
    for (size_t i = 0; i < ThreadNum; ++i) {
      serverThreads.emplace_back(std::thread([i]() { ThreadJob(i); }));
    }
    std::cout << "Server dispatched threads\n";

    for (auto& t : serverThreads) t.join();
    std::cout << "Server joined threads\n";
    std::cout << HashTable.GetStats(false) << "\n";
  }
  std::cout << "Server loop end\n";

  SemClose(semFull);
  SemClose(semEmpty);
  SemClose(semMutex);
  SemUnlink(SEM_FULL);
  SemUnlink(SEM_EMPTY);
  SemUnlink(SEM_MUTEX);

  MUnMap(buffer, FILE_SIZE);
  ShmClose(fd);
  ShmUnlink(FILE_NAME);

  return 0;
}
