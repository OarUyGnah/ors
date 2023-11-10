#include <cassert>
#include <chrono>
#include <iostream>
#include <time.h>
#include <utils/string_util.h>
namespace ors {
namespace utils {
namespace time {

constexpr clockid_t STEADY_CLOCK_ID = CLOCK_MONOTONIC;

template <typename clock, typename duration>
struct timespec
make_timespec(const std::chrono::time_point<clock, duration> &when) {
  std::chrono::nanoseconds::rep nanosSinceEpoch =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          when.time_since_epoch())
          .count();
  struct timespec ts;
  ts.tv_sec = nanosSinceEpoch / 1000000000L;
  ts.tv_nsec = nanosSinceEpoch % 1000000000L;
  // tv_nsec must always be in range [0, 1e9)
  if (nanosSinceEpoch < 0) {
    ts.tv_sec -= 1;
    ts.tv_nsec += 1000000000L;
  }
  return ts;
}

struct system_clock {
  using duration = std::chrono::nanoseconds;
  using rep = duration::rep;
  using period = duration::period;
  using time_point = std::chrono::time_point<system_clock, duration>;
  static constexpr bool is_steady = false;
  static time_point now();
};

struct steady_clock {
  using duration = std::chrono::nanoseconds;
  using rep = duration::rep;
  using period = duration::period;
  using time_point = std::chrono::time_point<steady_clock, duration>;
  static constexpr bool is_steady = true;
  static time_point now();
};

template <typename _BaseClock> struct MockableClock {
  typedef _BaseClock BaseClock;
  typedef typename BaseClock::duration duration;
  typedef typename BaseClock::rep rep;
  typedef typename BaseClock::period period;
  typedef typename BaseClock::time_point time_point;

  // libstdc++ 4.7 renamed monotonic_clock to steady_clock to conform with
  // C++11. This file doesn't use a BaseClock from libstdc++ before 4.8, so
  // it's ok to just assume is_steady is present.
  static const bool is_steady = BaseClock::is_steady;

  static time_point now() {
    if (useMockValue)
      return mockValue;
    else
      return BaseClock::now();
  }

  static bool useMockValue;
  static time_point mockValue;

  /// RAII class to mock out the clock and then restore it.
  struct Mocker {
    explicit Mocker(time_point value = now()) {
      assert(!useMockValue);
      useMockValue = true;
      mockValue = value;
    }
    ~Mocker() { useMockValue = false; }
  };
};

} // namespace time
} // namespace utils
} // namespace ors