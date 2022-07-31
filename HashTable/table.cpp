#include "table.h"

#include <algorithm>
#include <sstream>

// size_t HashFunc(const Key& key) { return std::hash<Key>{}(key); }
size_t HashFunc(const Key& key) { return static_cast<size_t>(key) % MAX_SIZE; }

bool Table::FindHelper(const Key& key, bool lock) {
  const auto& hash = HashFunc(key);
  auto& list = table.at(hash);
  const auto& searchKey = [&list, &key]() {
    auto it = std::find(std::cbegin(list), std::cend(list), key);
    if (it == std::cend(list)) return false;
    return true;
  };
  if (lock) {
    ReadLock _r(mutexes.at(hash));
    return searchKey();
  }
  return searchKey();
}

bool Table::FindImpl(const Key& key) { return FindHelper(key, true); }

bool Table::InsertImpl(const Key& key) {
  const auto& hash = HashFunc(key);
  if (FindImpl(key)) return false;
  WriteLock _w(mutexes.at(hash));
  if (FindHelper(key, false)) return false;
  auto& list = table.at(hash);
  list.emplace_back(key);
  return true;
}

bool Table::RemoveImpl(const Key& key) {
  const auto& hash = HashFunc(key);
  if (!FindImpl(key)) return false;
  WriteLock _w(mutexes.at(hash));
  if (!FindHelper(key, false)) return false;
  auto& list = table.at(hash);
  list.erase(std::remove(std::begin(list), std::end(list), key),
             std::end(list));
  return true;
}

bool Table::Find(const Key& key) {
  const auto& ret = FindImpl(key);
  if (ret)
    findNum++;
  else
    findNotNum++;
  return ret;
};
bool Table::Insert(const Key& key) {
  const auto& ret = InsertImpl(key);
  if (ret)
    insertNum++;
  else
    insertNotNum++;
  return ret;
}
bool Table::Remove(const Key& key) {
  const auto& ret = RemoveImpl(key);
  if (ret)
    removeNum++;
  else
    removeNotNum++;
  return ret;
}

std::string Table::GetStats(bool getContents) const {
  std::stringstream ss;
  if (getContents) {
    ss << "Table Contents\n";
    for (size_t hash = 0; hash < std::size(table); ++hash) {
      if (std::size(table.at(hash)) == 0) continue;
      ss << hash << " :";
      for (const auto& key : table.at(hash)) ss << " " << key;
      ss << "\n";
    }
  }
  ss << "Table Stats\n";
  ss << "(Find)\t\tSuccess: " << findNum << " Fail: " << findNotNum
     << " Total: " << findNum + findNotNum << "\n";
  ss << "(Insert)\tSuccess: " << insertNum << " Fail: " << insertNotNum
     << " Total: " << insertNum + insertNotNum << "\n";
  ss << "(Remove)\tSuccess: " << removeNum << " Fail: " << removeNotNum
     << " Total: " << removeNum + removeNotNum << "\n";
  return ss.str();
}
