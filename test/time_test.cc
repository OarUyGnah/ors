#include <gtest/gtest.h>
#include <iostream>
// #include <spdlog/spdlog.h>
#include <stdexcept>
#include <utils/string_util.h>
#include <utils/time.h>
namespace ors {
namespace utils {
using std::chrono::nanoseconds;
using string_util::to_string;

TEST(time, make_timespec) {
  struct timespec s;
  s = time::make_timespec(time::sys_clock::time_point::max());
  EXPECT_EQ(9223372036, s.tv_sec);
  EXPECT_EQ(854775807, s.tv_nsec);
  s = time::make_timespec(time::sys_clock::time_point::min());
  EXPECT_EQ(-9223372037, s.tv_sec);
  EXPECT_EQ(145224192, s.tv_nsec);
  s = time::make_timespec(time::sys_clock::now());
  EXPECT_LT(1417720382U, s.tv_sec); // 2014-12-04
  EXPECT_GT(1893456000U, s.tv_sec); // 2030-01-01
  s = time::make_timespec(time::sys_clock::time_point() +
                          std::chrono::nanoseconds(50));
  EXPECT_EQ(0, s.tv_sec);
  EXPECT_EQ(50, s.tv_nsec);
  s = time::make_timespec(time::sys_clock::time_point() -
                          std::chrono::nanoseconds(50));
  EXPECT_EQ(-1, s.tv_sec);
  EXPECT_EQ(999999950, s.tv_nsec);
}

TEST(time, clock_nanosecond_accuracy) {
  std::chrono::nanoseconds::rep nanos =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          time::system_clock::now().time_since_epoch())
          .count();
  std::chrono::nanoseconds::rep nanos2 =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          time::system_clock::now().time_since_epoch())
          .count();
  if (nanos % 1000 == 0) { // second try
    nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
                time::system_clock::now().time_since_epoch())
                .count();
  }
  EXPECT_LT(0, nanos % 1000);
  std::cout << "nanos\t: " << nanos << "\nnanos2\t: " << nanos2 << std::endl;
  EXPECT_NE(nanos, nanos2);

  std::chrono::nanoseconds::rep nanos3 =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          time::steady_clock::now().time_since_epoch())
          .count();
  if (nanos % 1000 == 0) { // second try
    nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
                time::steady_clock::now().time_since_epoch())
                .count();
  }
  EXPECT_LT(0, nanos % 1000);
}

TEST(time, sys_clock_now_diff) {
  time::sys_clock::time_point a = time::sys_clock::now();
  time::sys_clock::time_point b = time::sys_clock::now();
  EXPECT_LT(a, b);
  time::stdy_clock::time_point c = time::stdy_clock::now();
  time::stdy_clock::time_point d = time::stdy_clock::now();
  EXPECT_LT(c, d);
}

TEST(time, sys_clock_now_sensitive) {
  time::sys_clock::time_point a = time::sys_clock::now();
  usleep(1000);
  time::sys_clock::time_point b = time::sys_clock::now();
  EXPECT_LT(a, b);
  EXPECT_LT(a + std::chrono::microseconds(900), b);
  EXPECT_LT(b, a + std::chrono::microseconds(1100));

  time::stdy_clock::time_point c = time::stdy_clock::now();
  usleep(1000);
  time::stdy_clock::time_point d = time::stdy_clock::now();
  EXPECT_LT(c, d);
  EXPECT_LT(c + std::chrono::microseconds(900), d);
  EXPECT_LT(d, c + std::chrono::microseconds(1100));
}

TEST(time, parse_normal) {
  EXPECT_EQ(10000000000L, time::parse("10s"));
  EXPECT_EQ(182000000L, time::parse("182ms"));
  EXPECT_EQ(9000L, time::parse("9us"));
  EXPECT_EQ(9000L, time::parse("9 us "));
  EXPECT_EQ(10L, time::parse("10ns"));
  EXPECT_EQ(10L, time::parse("10ns"));
  EXPECT_EQ(0L, time::parse("0s"));
  EXPECT_EQ(0L, time::parse("0"));
  EXPECT_THROW(time::parse("10e"), ors::utils::time::invalid_time_description);
  EXPECT_THROW(time::parse("10 seconds now"),
               ors::utils::time::invalid_time_description);
  EXPECT_THROW(time::parse(""), ors::utils::time::invalid_time_description);
  EXPECT_THROW(time::parse(" "), ors::utils::time::invalid_time_description);
}

TEST(time, parse_overflow) {
  int64_t nearly = 1L << 62L;
  EXPECT_LT(nearly, time::parse("9223372036854775807 nanoseconds"));
  EXPECT_EQ(std::numeric_limits<int64_t>::max(),
            time::parse("9223372036854775808 nanoseconds"));
  EXPECT_LT(nearly, time::parse("9223372036854775 microseconds"));
  EXPECT_EQ(std::numeric_limits<int64_t>::max(),
            time::parse("9223372036854776 microseconds"));
  EXPECT_LT(nearly, time::parse("9223372036854 milliseconds"));
  EXPECT_EQ(std::numeric_limits<int64_t>::max(),
            time::parse("9223372036855 milliseconds"));
  EXPECT_LT(nearly, time::parse("9223372036 seconds"));
  EXPECT_EQ(std::numeric_limits<int64_t>::max(),
            time::parse("9223372037 seconds"));
  EXPECT_LT(nearly, time::parse("153722867 minutes"));
  EXPECT_EQ(std::numeric_limits<int64_t>::max(),
            time::parse("153722868 minutes"));
  EXPECT_LT(nearly, time::parse("2562047 hours"));
  EXPECT_EQ(std::numeric_limits<int64_t>::max(), time::parse("2562048 hours"));
  EXPECT_LT(nearly, time::parse("106751 days"));
  EXPECT_EQ(std::numeric_limits<int64_t>::max(), time::parse("106752 days"));
  EXPECT_LT(nearly, time::parse("15250 weeks"));
  EXPECT_EQ(std::numeric_limits<int64_t>::max(), time::parse("15251 weeks"));
  EXPECT_LT(nearly, time::parse("3507 months"));
  EXPECT_EQ(std::numeric_limits<int64_t>::max(), time::parse("3508 months"));
  EXPECT_LT(nearly, time::parse("292 years"));
  EXPECT_EQ(std::numeric_limits<int64_t>::max(), time::parse("293 years"));
  // 负值
  nearly = (1L << 62L) * -1L;
  EXPECT_GT(nearly, time::parse("-9223372036854775808 nanoseconds"));
  EXPECT_EQ(std::numeric_limits<int64_t>::min(),
            time::parse("-9223372036854775809 nanoseconds"));
  EXPECT_GT(nearly, time::parse("-9223372036854775 microseconds"));
  EXPECT_EQ(std::numeric_limits<int64_t>::min(),
            time::parse("-9223372036854776 microseconds"));
  EXPECT_GT(nearly, time::parse("-9223372036854 milliseconds"));
  EXPECT_EQ(std::numeric_limits<int64_t>::min(),
            time::parse("-9223372036855 milliseconds"));
  EXPECT_GT(nearly, time::parse("-9223372036 seconds"));
  EXPECT_EQ(std::numeric_limits<int64_t>::min(),
            time::parse("-9223372037 seconds"));
  EXPECT_GT(nearly, time::parse("-153722867 minutes"));
  EXPECT_EQ(std::numeric_limits<int64_t>::min(),
            time::parse("-153722868 minutes"));
  EXPECT_GT(nearly, time::parse("-2562047 hours"));
  EXPECT_EQ(std::numeric_limits<int64_t>::min(), time::parse("-2562048 hours"));
  EXPECT_GT(nearly, time::parse("-106751 days"));
  EXPECT_EQ(std::numeric_limits<int64_t>::min(), time::parse("-106752 days"));
  EXPECT_GT(nearly, time::parse("-15250 weeks"));
  EXPECT_EQ(std::numeric_limits<int64_t>::min(), time::parse("-15251 weeks"));
  EXPECT_GT(nearly, time::parse("-3507 months"));
  EXPECT_EQ(std::numeric_limits<int64_t>::min(), time::parse("-3508 months"));
  EXPECT_GT(nearly, time::parse("-292 years"));
  EXPECT_EQ(std::numeric_limits<int64_t>::min(), time::parse("-293 years"));

  EXPECT_EQ(2 * 31557600000000000UL,
            time::parse_non_negative_duration("2 year"));
  EXPECT_EQ(0UL, time::parse_non_negative_duration("0"));
  EXPECT_THROW(time::parse_non_negative_duration("-1 year"),
               ors::utils::time::invalid_time_description);
}

TEST(time, rdtsc) {
  uint64_t a = time::rdtsc();
  uint64_t b = time::rdtsc();
  std::cout << "a\t: " << a << " b\t: " << b << std::endl;
  EXPECT_LT(a, b);
}

// TEST(CoreTime, rdtsc_progressTimingSensitive) {
//   uint64_t a = Time::rdtsc();
//   usleep(1000);
//   uint64_t b = Time::rdtsc();
//   EXPECT_LT(a, b);
// #if defined(__i386) || defined(__x86_64__)
//   EXPECT_LT(a + 1000 * 1000, b);
// #elif defined(__powerpc64__)
//   EXPECT_LT(a + 1000 * 500, b);
// #else
// #error "Unsupported platform."
// #endif
//   EXPECT_LT(b, a + 10 * 1000 * 1000);
// }

TEST(time, sleep_tp) {
  time::steady_clock::time_point start = time::steady_clock::now();
  time::sleep(time::steady_clock::time_point::min());
  time::sleep(time::steady_clock::time_point());
  time::sleep(time::steady_clock::now() - std::chrono::milliseconds(1));
  time::sleep(time::steady_clock::now());
  EXPECT_GT(start + std::chrono::milliseconds(5), time::steady_clock::now());

  time::steady_clock::time_point start1 = time::steady_clock::now();
  time::sleep(start1 + std::chrono::milliseconds(12));
  time::steady_clock::time_point end1 = time::steady_clock::now();
  EXPECT_LT(start1 + std::chrono::milliseconds(12), end1);
  EXPECT_GT(start1 + std::chrono::milliseconds(17), end1);
}

TEST(time, sleep_duration) {
  time::steady_clock::time_point start = time::steady_clock::now();
  time::sleep(std::chrono::nanoseconds::min());
  time::sleep(std::chrono::nanoseconds(-10));
  time::sleep(std::chrono::nanoseconds(0));
  time::sleep(std::chrono::nanoseconds(10));
  EXPECT_GT(start + std::chrono::milliseconds(5), time::steady_clock::now());

  time::steady_clock::time_point start1 = time::steady_clock::now();
  time::sleep(std::chrono::milliseconds(12));
  time::steady_clock::time_point end1 = time::steady_clock::now();
  EXPECT_LT(start1 + std::chrono::milliseconds(12), end1);
  EXPECT_GT(start1 + std::chrono::milliseconds(17), end1);
}

TEST(time, sleepRelative_later_TimingSensitive) {
  
}

TEST(time, convert) {
  time::steady_converter conv;
  EXPECT_GT(time::system_clock::time_point(),
            conv.convert(time::steady_clock::time_point::min()));
  EXPECT_EQ(time::system_clock::time_point::max(),
            conv.convert(time::steady_clock::time_point::max()));
  EXPECT_EQ(time::system_clock::time_point::max(),
            conv.convert(time::steady_clock::time_point::max() -
                         std::chrono::hours(1)));
  EXPECT_LT(time::system_clock::time_point::min(),
            conv.convert(time::steady_clock::time_point::min() +
                         std::chrono::hours(1)));
  EXPECT_GT(time::system_clock::time_point(),
            conv.convert(time::steady_clock::time_point::min() +
                         std::chrono::hours(1)));
}

TEST(time, converter_nanos) {
  time::steady_converter conv;
  int64_t hour = 60L * 60 * 1000 * 1000 * 1000;
  EXPECT_GT(0, conv.nanos(time::steady_clock::time_point::min()));
  EXPECT_EQ(INT64_MAX, conv.nanos(time::steady_clock::time_point::max()));
  EXPECT_EQ(INT64_MAX, conv.nanos(time::steady_clock::time_point::max() -
                                  std::chrono::hours(1)));
  EXPECT_LT(INT64_MIN, conv.nanos(time::steady_clock::time_point::min() +
                                  std::chrono::hours(1)));
  EXPECT_GT(0, conv.nanos(time::steady_clock::time_point::min() +
                          std::chrono::hours(1)));
}

TEST(time, pad_fraction) {
  EXPECT_EQ("5 s", to_string(nanoseconds(5000000000)));
  EXPECT_EQ("-5 s", to_string(nanoseconds(-5000000000)));

  EXPECT_EQ("1.000000100 s", to_string(nanoseconds(1000000100)));
  EXPECT_EQ("2.123456700 s", to_string(nanoseconds(2123456700)));
  EXPECT_EQ("3.000000123 s", to_string(nanoseconds(3000000123)));
  EXPECT_EQ("-4.000000100 s", to_string(nanoseconds(-4000000100)));

  EXPECT_EQ("5.000000010 s", to_string(nanoseconds(5000000010)));
  EXPECT_EQ("6.123456780 s", to_string(nanoseconds(6123456780)));
  EXPECT_EQ("7.000000012 s", to_string(nanoseconds(7000000012)));
  EXPECT_EQ("-8.000000010 s", to_string(nanoseconds(-8000000010)));

  EXPECT_EQ("9.000000001 s", to_string(nanoseconds(9000000001)));
  EXPECT_EQ("10.123456789 s", to_string(nanoseconds(10123456789)));
  EXPECT_EQ("11.000000001 s", to_string(nanoseconds(11000000001)));
  EXPECT_EQ("-12.000000001 s", to_string(nanoseconds(-12000000001)));
}

TEST(time, output_string) {
  EXPECT_EQ("0 ns", to_string(std::chrono::nanoseconds(0)));
  EXPECT_EQ("5 ns", to_string(std::chrono::nanoseconds(5)));
  EXPECT_EQ("5 us", to_string(std::chrono::nanoseconds(5000)));
  EXPECT_EQ("5.001 us", to_string(std::chrono::nanoseconds(5001)));
  EXPECT_EQ("5 us", to_string(std::chrono::microseconds(5)));
  EXPECT_EQ("5 s", to_string(std::chrono::microseconds(5000000)));
  EXPECT_EQ("5.000001 s", to_string(std::chrono::microseconds(5000001)));
  EXPECT_EQ("5 s", to_string(std::chrono::milliseconds(5000)));
  EXPECT_EQ("5.001 s", to_string(std::chrono::milliseconds(5001)));
  EXPECT_EQ("18000 s", to_string(std::chrono::hours(5)));
  EXPECT_EQ("-3.001 us", to_string(std::chrono::nanoseconds(-3001)));
  EXPECT_EQ("time_point::min()", to_string(time::steady_clock::time_point::min()));
  std::cout << time::steady_clock::time_point::min() << std::endl;
  EXPECT_EQ("time_point::max()", to_string(time::system_clock::time_point::max()));
  EXPECT_LT(0.0, std::stold(to_string(time::system_clock::now())));
}

} // namespace utils
} // namespace ors