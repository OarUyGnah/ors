#ifndef __ORS_UTILS_COND_H__
#define __ORS_UTILS_COND_H__

#include <atomic>
#include <functional>
#include <memory>
#include <pthread.h>

#include <utils/mutex.h>
#include <utils/time.h>
namespace ors {
namespace utils {
class condition_variable {
public:
  enum notify_type{ ONE, ALL };

public:
  condition_variable();
  ~condition_variable();
  void notify(notify_type);
  void notify_one();
  void notify_all();
  void wait(std::unique_lock<std::mutex> &ul);
  void wait(std::unique_lock<mutex> &ul);

  // std::mutex and SteadyClock
  void wait_until(std::unique_lock<std::mutex> &ul,
                  const time::steady_clock::time_point &abs_time);

  // std::mutex and any clock: calls std::mutex and SteadyClock variant
  template <typename clock, typename duration>
  void wait_until(std::unique_lock<std::mutex> &ul,
                  const std::chrono::time_point<clock, duration> &abs_time) {
    std::chrono::time_point<clock, duration> now = clock::now();
    std::chrono::time_point<clock, duration> wake = abs_time;
    if (abs_time < now)
      wake = now - std::chrono::hours(1);
    else if (abs_time > now + std::chrono::hours(1))
      wake = now + std::chrono::hours(1);
    time::steady_clock::time_point steadyNow = time::steady_clock::now();
    time::steady_clock::time_point steadyWake = steadyNow + (wake - now);
    wait_until(ul, steadyWake);
  }

  // Core::Mutex and any clock: calls std::mutex and any clock variant
  template <typename clock, typename duration>
  void wait_until(std::unique_lock<mutex> &ul,
                  const std::chrono::time_point<clock, duration> &abs_time) {
    mutex &mtx(*ul.mutex());
    if (mtx.cb)
      mtx.cb();
    assert(ul);
    std::unique_lock<std::mutex> stdLockGuard(mtx.mtx, std::adopt_lock_t());
    ul.release();
    wait_until(stdLockGuard, abs_time);
    assert(stdLockGuard);
    ul = std::unique_lock<mutex>(mtx, std::adopt_lock_t());
    stdLockGuard.release();
    if (mtx.cb)
      mtx.cb();
  }

private:
  void init();

  // private:
public:
  pthread_cond_t cv;

  std::function<void()> cb;

public:
  std::atomic<uint64_t> notify_count;

  time::steady_clock::time_point last_wu;
};
} // namespace utils
} // namespace ors

#endif //!__ORS_UTILS_COND_H__