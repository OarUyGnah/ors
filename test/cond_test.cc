#include <string>
#include <thread>
// #include <unordered_map>
// #include <unordered_set>
// #include <mutex>
// #include <utils/tid.h>
#include <gtest/gtest.h>
// #include <utils/mutex.h>
#include <spdlog/spdlog.h>
#include <utils/cond.h>
#include <utils/time.h>

typedef std::chrono::milliseconds ms;
namespace ors {
namespace utils {
class cond_test : public ::testing::Test {
public:
  cond_test()
      : mutex(), stdmutex(), cv(), ready(0), done(0), counter1(0), counter2(0),
        mutexCounter(0), cond(false), thread1(), thread2() {
    mutex.cb = std::bind(&cond_test::incrementMutexCounter, this);
  }
  ~cond_test() {
    cv.notify_all();
    if (thread1.joinable())
      thread1.join();
    if (thread2.joinable())
      thread2.join();
  }

  void wait() {
    std::unique_lock<utils::mutex> lg(mutex);
    ++ready;
    while (!cond) {
      cv.wait(lg);
      ++counter2;
    }
    ++done;
  }

  void waitStdMutex() {
    std::unique_lock<std::mutex> lg(stdmutex);
    ++ready;
    while (!cond) {
      cv.wait(lg);
      ++counter2;
    }
    ++done;
  }

  void incrementCounter1() { ++counter1; }

  void incrementCounter2() { ++counter2; }

  void incrementMutexCounter() { ++mutexCounter; }

  void spinForReady(uint64_t until) {
    for (uint64_t i = 0; i < 100; ++i) {
      if (ready >= until)
        return;
      usleep(1000);
    }
    spdlog::error("timeout");
  }

  void sleepAndNotify(uint64_t ms, std::atomic<bool> &exit) {
    for (uint64_t i = 0; i < ms; ++i) {
      if (exit)
        return;
      usleep(1000);
    }
    cv.notify_all();
  }

  template <typename LockGuard, typename Clock, typename Duration>
  Duration
  timedWaitUntil(LockGuard &lg,
                 const std::chrono::time_point<Clock, Duration> &abs_time,
                 uint64_t watchdogMs = 50) {
    std::atomic<bool> watchdogExit(false);
    std::thread watchdog(&cond_test::sleepAndNotify, this, watchdogMs,
                         std::ref(watchdogExit));
    typename Clock::time_point start = Clock::now();
    cv.wait_until(lg, abs_time);
    typename Clock::time_point end = Clock::now();
    watchdogExit = true;
    watchdog.join();
    return end - start;
  }

  utils::mutex mutex;
  std::mutex stdmutex;
  condition_variable cv;
  std::atomic<uint64_t> ready;
  std::atomic<uint64_t> done;
  std::atomic<uint64_t> counter1;
  std::atomic<uint64_t> counter2;
  std::atomic<uint64_t> mutexCounter;
  std::atomic<bool> cond;
  std::thread thread1;
  std::thread thread2;
};

TEST_F(cond_test, notify_one_TimingSensitive) {
  cv.notify(condition_variable::ONE);
  EXPECT_EQ(1U, cv.notify_count);
  thread1 = std::thread(&cond_test::wait, this);
  thread2 = std::thread(&cond_test::wait, this);
  spinForReady(2);
  EXPECT_EQ(0U, done);
  {
    std::unique_lock<utils::mutex> lg(mutex);
    cond = true;
    cv.notify(condition_variable::ONE);
  }
  EXPECT_EQ(2U, cv.notify_count);
  usleep(1000);
  EXPECT_EQ(1U, counter2);
  EXPECT_EQ(1U, done);
  {
    std::unique_lock<utils::mutex> lg(mutex);
    cond = true;
    cv.notify(condition_variable::ONE);
  }
  usleep(1000);
  EXPECT_EQ(2U, counter2);
  EXPECT_EQ(2U, done);
  thread1.join();
  thread2.join();
}

TEST_F(cond_test, notify_all_TimingSensitive) {
  cv.notify(condition_variable::ALL);
  EXPECT_EQ(1U, cv.notify_count);
  thread1 = std::thread(&cond_test::wait, this);
  thread2 = std::thread(&cond_test::wait, this);
  spinForReady(2);
  EXPECT_EQ(0U, done);
  {
    std::unique_lock<utils::mutex> lg(mutex);
    cond = true;
    cv.notify(condition_variable::ALL);
  }
  EXPECT_EQ(2U, cv.notify_count);
  thread1.join();
  thread2.join();
  EXPECT_EQ(2U, counter2);
  EXPECT_EQ(2U, done);
}

TEST_F(cond_test, wait_stdmutex_callback) {
  cv.cb = std::bind(&cond_test::incrementCounter1, this);
  std::unique_lock<std::mutex> lg(stdmutex);
  cv.wait(lg);
  EXPECT_EQ(1U, counter1);
}

TEST_F(cond_test, wait_stdmutex_real_TimingSensitive) {
  thread1 = std::thread(&cond_test::waitStdMutex, this);
  spinForReady(1);
  EXPECT_EQ(0U, done);
  {
    std::unique_lock<std::mutex> lg(stdmutex);
    cond = true;
    cv.notify_all();
  }
  thread1.join();
  EXPECT_EQ(1U, counter2);
}

TEST_F(cond_test, wait_mutex_callback) {
  cv.cb = std::bind(&cond_test::incrementCounter1, this);
  {
    std::unique_lock<utils::mutex> lg(mutex);
    cv.wait(lg);
  }
  EXPECT_EQ(1U, counter1);
  EXPECT_EQ(4U, mutexCounter); // 2 from lock guard, 2 from wait
}

TEST_F(cond_test, wait_mutex_realtime_sensitive) {
  thread1 = std::thread(&cond_test::wait, this);
  spinForReady(1);
  EXPECT_EQ(0U, done);
  {
    std::unique_lock<utils::mutex> lg(mutex);
    cond = true;
    cv.notify_all();
  }
  thread1.join();
  EXPECT_EQ(1U, counter2);
  EXPECT_EQ(6U, mutexCounter); // 4 from lock guards, 2 from wait

  std::unique_lock<std::mutex> lg2(stdmutex);
  EXPECT_LT(timedWaitUntil(lg2, time::steady_clock::time_point::min()), ms(1));
  EXPECT_LT(timedWaitUntil(lg2, time::steady_clock::now() - ms(1)), ms(1));
  EXPECT_LT(timedWaitUntil(lg2, time::steady_clock::now()), ms(1));

  EXPECT_GT(timedWaitUntil(lg2, time::steady_clock::time_point::max(), 2),
            ms(1));
}

} // namespace utils
} // namespace ors
