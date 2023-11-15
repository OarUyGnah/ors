#ifndef __ORS_UTILS_TIME_H__
#define __ORS_UTILS_TIME_H__

#include <cassert>
#include <chrono>
#include <exception>
#include <iostream>
#include <time.h>
#include <utils/common.h>
namespace ors {
namespace utils {
namespace time {

struct stdy_clock;
// typename stdy_clock::time_point;
struct sys_clock;
template <typename baseclock> struct mock_clock;
struct invalid_time_description;

#if __clang__ || (__GNUC__ == 4 && __GNUC_MINOR__ < 8)
using steady_clock = typename mock_clock<stdy_clock>;
using system_clock = typename mock_clock<sys_clock>;
#else
using steady_clock =
    typename ors::utils::time::mock_clock<std::chrono::steady_clock>;
using system_clock =
    typename ors::utils::time::mock_clock<std::chrono::system_clock>;
#endif

constexpr clockid_t STEADY_CLOCK_ID = CLOCK_MONOTONIC;

template <typename clock, typename duration>
struct timespec
make_timespec(const std::chrono::time_point<clock, duration> &tp) {
  std::chrono::nanoseconds::rep nanosSinceEpoch =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          tp.time_since_epoch())
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

struct stdy_clock {
  using duration = std::chrono::nanoseconds;
  using rep = typename duration::rep;
  using period = typename duration::period;
  using time_point = typename std::chrono::time_point<stdy_clock, duration>;
  static constexpr bool is_steady = true;
  static time_point now();
};

struct sys_clock {
  using duration = std::chrono::nanoseconds;
  using rep = typename duration::rep;
  using period = typename duration::period;
  using time_point = typename std::chrono::time_point<sys_clock, duration>;
  static constexpr bool is_steady = false;
  static time_point now();
};

template <typename baseclock> struct mock_clock {
  // typedef clock clock;
  // typedef typename clock::duration duration;
  // typedef typename clock::rep rep;
  // typedef typename clock::period period;
  // typedef typename clock::time_point time_point;
  using clock = baseclock;
  using duration = typename clock::duration;
  using rep = typename clock::rep;
  using period = typename clock::period;
  using time_point = typename clock::time_point;

  // libstdc++ 4.7 renamed monotonic_clock to steady_clock to conform with
  // C++11. This file doesn't use a BaseClock from libstdc++ before 4.8, so
  // it's ok to just assume is_steady is present.
  static constexpr bool is_steady = clock::is_steady;

  static bool is_mock_value;

  static time_point mock_value;

  static time_point now() { return is_mock_value ? mock_value : clock::now(); }

  /// RAII class to mock out the clock and then restore it.
  struct mocker {
    explicit mocker(time_point value = now()) {
      assert(!is_mock_value);
      is_mock_value = true;
      mock_value = value;
    }
    ~mocker() { is_mock_value = false; }
  };
};

template <typename baseclock> bool mock_clock<baseclock>::is_mock_value = false;

template <typename baseclock>
typename mock_clock<baseclock>::time_point mock_clock<baseclock>::mock_value;

int64_t parse(const std::string &description);

uint64_t parse_non_negative_duration(const std::string &description);

void sleep(ors::utils::time::steady_clock::time_point wake);

void sleep(std::chrono::nanoseconds duration);

class steady_converter {
public:
  steady_converter();

  system_clock::time_point convert(steady_clock::time_point tp);

  int64_t nanos(steady_clock::time_point tp);

private:
// public:
  steady_clock::time_point stdy_tp;

  system_clock::time_point sys_tp;
};

struct invalid_time_description : std::exception {
  invalid_time_description() {}
  explicit invalid_time_description(const std::string &msg) : msg(msg) {}
  virtual ~invalid_time_description() throw(){};
  std::string msg;
};
// 获取处理器的时间戳计数 tsc
static __inline __attribute__((always_inline)) uint64_t rdtsc() {
#if defined(__i386) || defined(__x86_64__)
  uint32_t low, high;
  // 低32位放在low 高32位放在high
  __asm__ __volatile__("rdtsc" : "=a"(low), "=d"(high));
  return ((uint64_t(high) << 32) | low);
#elif defined(__powerpc64__)
  return (__builtin_ppc_get_timebase());
#else
#error "Unsupported platform."
#endif
}

} // namespace time
} // namespace utils
} // namespace ors

namespace std {

std::ostream &operator<<(std::ostream &os,
                         const std::chrono::nanoseconds &duration);
std::ostream &operator<<(std::ostream &os,
                         const std::chrono::microseconds &duration);
std::ostream &operator<<(std::ostream &os,
                         const std::chrono::milliseconds &duration);
std::ostream &operator<<(std::ostream &os,
                         const std::chrono::seconds &duration);
std::ostream &operator<<(std::ostream &os,
                         const std::chrono::minutes &duration);
std::ostream &operator<<(std::ostream &os, const std::chrono::hours &duration);

/**
 * Prints std::time_point values in a way that is useful for unit tests.
 */
template <typename clock, typename duration>
std::ostream &operator<<(std::ostream &os,
                         const std::chrono::time_point<clock, duration> &tp) {
  if (std::chrono::time_point<clock, duration>::min() == tp)
    return os << "time_point::min()";
  if (std::chrono::time_point<clock, duration>::max() == tp)
    return os << "time_point::max()";
  struct timespec ts = ors::utils::time::make_timespec(tp);
  return os << ors::utils::string::fmt("%ld.%09ld", ts.tv_sec, ts.tv_nsec);
}

} // namespace std

#endif // !__ORS_UTILS_TIME_H__