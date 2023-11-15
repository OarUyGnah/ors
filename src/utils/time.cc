#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <stdexcept>

#include <spdlog/spdlog.h>
#include <utils/time.h>

namespace ors {
namespace utils {
namespace time {
int64_t parse(const std::string &description) {
  const char *start = description.c_str();
  char *end = nullptr;
  errno = 0;
  int64_t r = strtol(start, &end, 10);
  if (errno == ERANGE) {
    // pass
  } else if (errno != 0 || start == end) {
    throw invalid_time_description(
        ors::utils::string::fmt("Invalid time description: "
                                     "could not parse number from \"%s\"",
                                     description.c_str()));
  }

  std::string units = end;
  units.erase(std::find_if(units.rbegin(), units.rend(),
                           std::not1(std::ptr_fun<int, int>(std::isspace)))
                  .base(),
              units.end());
  units.erase(units.begin(),
              std::find_if(units.begin(), units.end(),
                           std::not1(std::ptr_fun<int, int>(std::isspace))));

  int64_t overflow;
  if (r < 0L)
    overflow = std::numeric_limits<int64_t>::min();
  else
    overflow = std::numeric_limits<int64_t>::max();

  if (units == "ns" || units == "nanosecond" || units == "nanoseconds") {
    // pass
  } else if (units == "us" || units == "microsecond" ||
             units == "microseconds") {
    if (std::abs(r) <= 9223372036854775L)
      r *= 1000L;
    else
      r = overflow;
  } else if (units == "ms" || units == "millisecond" ||
             units == "milliseconds") {
    if (std::abs(r) <= 9223372036854L)
      r *= 1000000L;
    else
      r = overflow;
  } else if (units == "s" || units == "second" || units == "seconds" ||
             units == "") {
    if (std::abs(r) <= 9223372036L)
      r *= 1000000000L;
    else
      r = overflow;
  } else if (units == "min" || units == "minute" || units == "minutes") {
    if (std::abs(r) <= 153722867L)
      r *= 1000000000L * 60L;
    else
      r = overflow;
  } else if (units == "h" || units == "hr" || units == "hour" ||
             units == "hours") {
    if (std::abs(r) <= 2562047L)
      r *= 1000000000L * 60L * 60L;
    else
      r = overflow;
  } else if (units == "d" || units == "day" || units == "days") {
    if (std::abs(r) <= 106751L)
      r *= 1000000000L * 60L * 60L * 24L;
    else
      r = overflow;
  } else if (units == "w" || units == "wk" || units == "week" ||
             units == "weeks") {
    if (std::abs(r) <= 15250L)
      r *= 1000000000L * 60L * 60L * 24L * 7L;
    else
      r = overflow;
  } else if (units == "mo" || units == "month" || units == "months") {
    // Months vary in length, so this is the average number of seconds in a
    // month. If someone is specifying durations in such large units, they
    // probably won't care.
    if (std::abs(r) <= 3507L)
      r *= 1000000000L * 2629800L;
    else
      r = overflow;
  } else if (units == "y" || units == "yr" || units == "year" ||
             units == "years") {
    // Years vary in length due to leap years, so this is the number of
    // seconds in a 365.25-day year. If someone is specifying durations in
    // such large units, they probably won't care.
    if (std::abs(r) <= 292L)
      r *= 1000000000L * 31557600L;
    else
      r = overflow;
  } else {
    throw invalid_time_description(
        ors::utils::string::fmt("Invalid time description: "
                                     "could not parse units from \"%s\"",
                                     description.c_str()));
  }
  return r;
}

uint64_t parse_non_negative_duration(const std::string &description) {
  int64_t r = parse(description);
  if (r < 0) {
    throw invalid_time_description(ors::utils::string::fmt(
        "Invalid time description: \"%s\" is negative", description.c_str()));
  }
  return static_cast<uint64_t>(r);
}

void sleep(ors::utils::time::steady_clock::time_point wake) {
  struct timespec spec = make_timespec(wake);
  if (spec.tv_sec < 0)
    return;
  int r = clock_nanosleep(STEADY_CLOCK_ID, TIMER_ABSTIME, &spec, nullptr);
  if (r != 0) {
    SPDLOG_ERROR("clock_nanosleep(STEADY_CLOCK_ID=%d, %s) failed: %s",
                 STEADY_CLOCK_ID,
                 ors::utils::string::to_string(wake).c_str(), strerror(r));
  }
}

void sleep(std::chrono::nanoseconds duration) {
  if (duration <= std::chrono::nanoseconds::zero())
    return;
  steady_clock::time_point now = steady_clock::now();
  steady_clock::time_point wake = now + duration;
  if (wake < now) { // overflow
    wake = steady_clock::time_point::max();
  }
  sleep(wake);
}

stdy_clock::time_point stdy_clock::now() {
  struct timespec now;
  int r = clock_gettime(STEADY_CLOCK_ID, &now);
  if (r != 0) {
    SPDLOG_ERROR("clock_gettime(STEADY_CLOCK_ID) failed: %s", strerror(errno));
  }
  return time_point(std::chrono::nanoseconds(
      int64_t(now.tv_sec) * 1000 * 1000 * 1000 + now.tv_nsec));
}

sys_clock::time_point sys_clock::now() {
  struct timespec now;
  int r = clock_gettime(CLOCK_REALTIME, &now);
  if (r != 0) {
    SPDLOG_ERROR("clock_gettime(CLOCK_REALTIME) failed: %s", strerror(errno));
  }
  return time_point(std::chrono::nanoseconds(
      int64_t(now.tv_sec) * 1000 * 1000 * 1000 + now.tv_nsec));
}


steady_converter::steady_converter()
    : stdy_tp(steady_clock::now())
    , sys_tp(system_clock::now())
{
}

system_clock::time_point
steady_converter::convert(steady_clock::time_point tp) {
  std::chrono::nanoseconds gap = tp - stdy_tp;
  system_clock::time_point ret = sys_tp + gap;
  if (tp > stdy_tp && ret < sys_tp) // overflow
    return system_clock::time_point::max();
  return ret;
}

int64_t steady_converter::nanos(steady_clock::time_point tp) {
  return std::chrono::nanoseconds(convert(tp).time_since_epoch()).count();
}
} // namespace time
} // namespace utils
} // namespace ors

namespace std {

namespace {

std::string pad_fraction(int64_t fraction, uint64_t digits) {
  if (fraction == 0)
    return "";
  if (fraction < 0)
    fraction *= -1;
  while (fraction % 1000 == 0 && digits >= 3) {
    digits -= 3;
    fraction /= 1000;
  }
  char ret[digits + 2];
  ret[0] = '.';
  ret[digits + 1] = '\0';
  for (uint64_t i = digits; i > 0; --i) {
    ret[i] = char('0' + (fraction % 10));
    fraction /= 10;
  }
  return ret;
}

} // namespace

std::ostream &operator<<(std::ostream &os,
                         const std::chrono::nanoseconds &duration) {
  int64_t nanos = duration.count();
  if (nanos / 1000000000L != 0) {
    int64_t whole = nanos / 1000000000L;
    int64_t fraction = nanos % 1000000000L;
    os << whole << pad_fraction(fraction, 9) << " s";
  } else if (nanos / 1000000L != 0) {
    int64_t whole = nanos / 1000000L;
    int64_t fraction = nanos % 1000000L;
    os << whole << pad_fraction(fraction, 6) << " ms";
  } else if (nanos / 1000L != 0) {
    int64_t whole = nanos / 1000L;
    int64_t fraction = nanos % 1000L;
    os << whole << pad_fraction(fraction, 3) << " us";
  } else {
    os << nanos << " ns";
  }
  return os;
}

std::ostream &operator<<(std::ostream &os,
                         const std::chrono::microseconds &duration) {
  return os << chrono::duration_cast<chrono::nanoseconds>(duration);
}

std::ostream &operator<<(std::ostream &os,
                         const std::chrono::milliseconds &duration) {
  return os << chrono::duration_cast<chrono::nanoseconds>(duration);
}

std::ostream &operator<<(std::ostream &os,
                         const std::chrono::seconds &duration) {
  return os << chrono::duration_cast<chrono::nanoseconds>(duration);
}

std::ostream &operator<<(std::ostream &os,
                         const std::chrono::minutes &duration) {
  return os << chrono::duration_cast<chrono::nanoseconds>(duration);
}

std::ostream &operator<<(std::ostream &os, const std::chrono::hours &duration) {
  return os << chrono::duration_cast<chrono::nanoseconds>(duration);
}

} // namespace std