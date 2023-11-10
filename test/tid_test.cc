#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <utils/tid.h>
#include <gtest/gtest.h>

using namespace ors::utils;

namespace ors {
namespace utils {
namespace tid {
extern __thread uint64_t _tid;
extern std::atomic<uint64_t> _nextid;

extern std::map<uint64_t, std::string> _tid_name_map;
extern std::unordered_set<std::string> _tid_name_set;
} // namespace tid
} // namespace utils
} // namespace ors

class core_tid_test : public ::testing::Test {
public:
  core_tid_test() {
    tid::_tid = 0;
    tid::_nextid = 1;
    tid::_tid_name_map.clear();
    tid::_tid_name_set.clear();
  }
};

// Helper function that runs in a separate thread.  It reads its id and
// saves it in the variable pointed to by its argument.
static void read_tid(uint64_t *p) { *p = tid::tid(); }

TEST_F(core_tid_test, basics) {
  uint64_t value;
  EXPECT_EQ(1U, tid::tid());
  EXPECT_EQ(1U, tid::tid());
  std::thread thread1(read_tid, &value);
  thread1.join();
  EXPECT_EQ(2U, value);
  std::thread thread2(read_tid, &value);
  thread2.join();
  EXPECT_EQ(3U, value);
}

TEST_F(core_tid_test, names) {
  EXPECT_EQ("Thread 1", tid::name());
  tid::set_name("foo");
  EXPECT_EQ("foo", tid::name());
  tid::set_name("bar");
  EXPECT_EQ("bar", tid::name());
  tid::set_name("");
  EXPECT_EQ("Thread 1", tid::name());
}