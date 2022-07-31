#ifndef _TABLE_H_
#define _TABLE_H_

#include <atomic>
#include <list>
#include <map>
#include <memory>
#include <shared_mutex>

#include "utils.h"

constexpr size_t MAX_SIZE = 32 * 1024;
class Table {
 private:
  std::atomic<size_t> findNum{};
  std::atomic<size_t> findNotNum{};
  std::atomic<size_t> insertNum{};
  std::atomic<size_t> insertNotNum{};
  std::atomic<size_t> removeNum{};
  std::atomic<size_t> removeNotNum{};

 public:
  using SharedMutex = std::shared_mutex;
  using SharedMutexUPtr = std::unique_ptr<SharedMutex>;
  using ReadLock = std::shared_lock<SharedMutex>;
  using WriteLock = std::unique_lock<SharedMutex>;

 private:
  std::array<std::list<Key>, MAX_SIZE> table{};
  std::array<SharedMutex, MAX_SIZE> mutexes{};

  void InsertMutex(const size_t& hash);
  bool FindHelper(const Key& key, bool lock);

  bool FindImpl(const Key& key);
  bool InsertImpl(const Key& key);
  bool RemoveImpl(const Key& key);

 public:
  bool Find(const Key& key);
  bool Insert(const Key& key);
  bool Remove(const Key& key);
  std::string GetStats(bool getContents) const;
};

#endif
