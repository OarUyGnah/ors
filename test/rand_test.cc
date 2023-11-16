#include <gtest/gtest.h>
#include <thread>
#include <unistd.h>

// #include <utils/common.h>
#include <utils/rand.h>
#include <utils/cond.h>
namespace ors {
namespace utils {

namespace rand {
TEST(rand_test, fork) {
  // failures counts the number of attempts that the parent and child chose
  // the same random value. This is expected attempt/256 times.
  uint64_t failures = 0;
  for (uint64_t attempt = 0; attempt < 16; ++attempt) {
    errno = 0;
    pid_t pid = fork();
    ASSERT_NE(pid, -1) << strerror(errno); // error
    if (pid == 0) {                        // child
      _exit(random8());
    } else { // parent
      uint8_t parent = random8();
      int status = 0;
      int r = waitpid(pid, &status, 0);
      ASSERT_EQ(pid, r);
      ASSERT_TRUE(WIFEXITED(status));
      uint8_t child = uint8_t(WEXITSTATUS(status));
      if (parent == child)
        ++failures;
    }
  }
  EXPECT_GT(2U, failures) << failures;
}

TEST(rand_test, bit8) {
  uint8_t r = 0;
  for (uint32_t i = 0; i < 20; ++i)
    r = uint8_t(r | random8());
  EXPECT_EQ(0xFF, r);
}

TEST(rand_test, bit16) {
  uint16_t r = 0;
  for (uint32_t i = 0; i < 20; ++i)
    r = uint16_t(r | random16());
  EXPECT_EQ(0xFFFF, r);
}

TEST(rand_test, bit32) {
  uint32_t r = 0;
  for (uint32_t i = 0; i < 20; ++i)
    r = uint32_t(r | random32());
  EXPECT_EQ(~0U, r);
}

TEST(rand_test, bit64) {
  uint64_t r = 0;
  for (uint32_t i = 0; i < 20; ++i)
    r = uint64_t(r | random64());
  EXPECT_EQ(~0UL, r);
}

TEST(rand_test, random_range_double) {
  EXPECT_LT(0.0, random_range<double>(0.0, 1.0));
  EXPECT_GT(1.0, random_range<double>(0.0, 1.0));
  EXPECT_LT(2.0, random_range<double>(2.0, 3.0));
  EXPECT_GT(3.0, random_range<double>(2.0, 3.0));
  EXPECT_LT(1.5, random_range<double>(1.5, 1.6));
  EXPECT_GT(1.6, random_range<float>(1.5, 1.6));
  EXPECT_LT(-0.5, random_range<float>(-0.5, 0.5));
  EXPECT_GT(0.5, random_range<float>(-0.5, 0.5));
  EXPECT_EQ(10.0, random_range<float>(10.0, 10.0));
  EXPECT_NE(random_range<float>(0.0, 1.0), random_range<double>(0.0, 1.0));

  EXPECT_LT(0.0, random_range<double>(1.0, 0.0));
  EXPECT_GT(1.0, random_range<double>(1.0, 0.0));
  EXPECT_LT(2.0, random_range<double>(3.0, 2.0));
  EXPECT_GT(3.0, random_range<double>(3.0, 2.0));
  EXPECT_LT(1.5, random_range<double>(1.6, 1.5));
  EXPECT_GT(1.6, random_range<float>(1.6, 1.5));
  EXPECT_LT(-0.5, random_range<float>(0.5, -0.5));
  EXPECT_GT(0.5, random_range<float>(0.5, -0.5));
  EXPECT_EQ(10.0, random_range<float>(10.0, 10.0));
  EXPECT_NE(random_range<float>(1.0, 0.0), random_range<double>(1.0, 0.0));
}

TEST(CoreRandomTest, randomRangeInt) {
  uint64_t ones = 0;
  uint64_t twos = 0;
  for (uint32_t i = 0; i < 20; i++) {
    uint64_t r = random_range(1, 2);
    EXPECT_LE(1U, r);
    EXPECT_GE(2U, r);
    if (r == 1)
      ++ones;
    else if (r == 2)
      ++twos;
  }
  EXPECT_LT(0U, ones);
  EXPECT_LT(0U, twos);

  EXPECT_LE(0U, random_range(0, 10));
  EXPECT_GE(10U, random_range(0, 10));
  EXPECT_LE(20U, random_range(20, 30));
  EXPECT_GE(30U, random_range(20, 30));
  EXPECT_LE(15U, random_range(15, 16));
  EXPECT_GE(16U, random_range(15, 16));
  EXPECT_EQ(10U, random_range(10, 10));
  EXPECT_NE(random_range(0, 10000), random_range(0, 10000));

  EXPECT_LE(0U, random_range(10, 0));
  EXPECT_GE(10U, random_range(10, 0));
  EXPECT_LE(20U, random_range(30, 20));
  EXPECT_GE(30U, random_range(30, 20));
  EXPECT_LE(15U, random_range(16, 15));
  EXPECT_GE(16U, random_range(16, 15));
  EXPECT_EQ(10U, random_range(10, 10));
  EXPECT_NE(random_range(10000, 0), random_range(10000, 0));
}

void sleep_with_Lock(bool &haveLock, std::mutex &m,
                   ors::utils::condition_variable &c) {
  ors::utils::rand::acquire_mutex();
  {
    std::unique_lock<std::mutex> lock(m);
    haveLock = true;
    c.notify_one();
  }
  usleep(10000);
  ors::utils::rand::release_mutex();
}

TEST(rand_test, forkWillNotDeadlock_TimingSensitive) {
  bool haveLock(false);
  std::mutex m;
  utils::condition_variable c;
  std::thread t(&sleep_with_Lock, std::ref(haveLock), std::ref(m), std::ref(c));

  {
    std::unique_lock<std::mutex> lock(m);
    while (!haveLock) {
      c.wait(lock);
    }
  }

  pid_t pid = fork();
  if (pid == 0) { // child
    random8();
    _exit(0);
  } else { // parent
    random8();
    int status = 0;
    waitpid(pid, &status, 0);
    ASSERT_TRUE(WIFEXITED(status));
  }
  t.join();
  // if we exit this function without hanging we pass
}

} // namespace rand
} // namespace utils
} // namespace ors