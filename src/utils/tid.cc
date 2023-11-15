#include <atomic>
#include <map>
#include <mutex>
#include <unordered_set>
#include <utils/common.h>
#include <utils/tid.h>

// static std::atomic<uint64_t> _nextid = 1;
// static std::mutex _mtx;

// static std::map<uint64_t, std::string> _tid_name_map;
// static std::unordered_set<std::string> _tid_name_set;
namespace ors {
namespace utils {
namespace tid {
__thread uint64_t _tid = 0;

// static uint64_t nextid;
std::atomic<uint64_t> _nextid = 1;
std::mutex _mtx;

std::map<uint64_t, std::string> _tid_name_map;
std::unordered_set<std::string> _tid_name_set;

uint64_t tid() {
  if (_tid == 0) {
    std::lock_guard<std::mutex> lg(_mtx);
    _tid = _nextid;
    ++_nextid;
  }
  return _tid;
}

void set_name(std::string name) {
  auto curr_tid = tid();
  std::lock_guard<std::mutex> lg(_mtx);
  auto trim_name = ors::utils::string::trim(name);
  try {
    if (trim_name.size() == 0 || _tid_name_set.count(trim_name)) {
      throw thread_name_alredy_exist(trim_name);
    } else {
      _tid_name_map[curr_tid] = trim_name;
      _tid_name_set.insert(trim_name);
    }
  } catch (thread_name_alredy_exist &e) {
    // spdlog::warning()
    _tid_name_map[curr_tid] =
        std::move(string::fmt("Thread %ld", curr_tid));
  }

  //   else tid_name_map[curr_tid] = trim_name;
}

std::string name() {
  auto curr_tid = tid();
  std::lock_guard<std::mutex> lg(_mtx);
  if (_tid_name_map.count(curr_tid))
    return _tid_name_map[curr_tid];
  else
    return std::move(ors::utils::string::fmt("Thread %lu", curr_tid));
}
} // namespace tid
} // namespace utils
} // namespace ors