#include <atomic>
#include <map>
#include <mutex>
#include <unordered_set>
#include <utils/string_util.h>
#include <utils/tid.h>
__thread static uint64_t _tid = 0;

// static uint64_t nextid;
static std::atomic<uint64_t> nextid;
static std::mutex mtx;

static std::map<uint64_t, std::string> tid_name_map;
static std::unordered_set<std::string> tid_name_set;
namespace ors {
namespace utils {
namespace tid {

uint64_t tid() {
  if (_tid == 0) {
    std::lock_guard<std::mutex> lg(mtx);
    _tid = nextid;
    ++nextid;
  }
}

void set_name(std::string name) {
  auto curr_tid = tid();
  std::lock_guard<std::mutex> lg(mtx);
  auto trim_name = ors::utils::string_util::trim(name);
  try {
    if (trim_name.size() == 0 || tid_name_set.count(trim_name)) {
      throw thread_name_alredy_exist(trim_name);
    } else {
        tid_name_map[curr_tid] = trim_name;
        tid_name_set.insert(trim_name);
    }
  } catch (thread_name_alredy_exist &e) {
    // spdlog::warning()
    tid_name_map[curr_tid] = std::move(string_util::fmt("Thread %d",curr_tid));
  }

//   else tid_name_map[curr_tid] = trim_name;
}

std::string name() {
  auto curr_tid = tid();
  std::lock_guard<std::mutex> lg(mtx);
  if (tid_name_map.count(curr_tid))
    return tid_name_map[curr_tid];
  else
    return std::move(ors::utils::string_util::fmt("thread %lu", curr_tid));
}
} // namespace tid
} // namespace utils
} // namespace ors