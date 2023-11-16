#ifndef __ORS_UTILS_RAND_H__
#define __ORS_UTILS_RAND_H__
#include <cinttypes>
#include <mutex>
#include <random>

namespace ors {
namespace utils {
namespace rand {

void acquire_mutex();
void release_mutex();
void reset_random_state();

class prng {
public:
  prng();

  ~prng();

  void set_distribution(uint64_t begin, uint64_t end);

  void reset_seed();

  uint64_t random64();

  std::mutex &mutex();

  friend void acquire_mutex();
  friend void acquire_mutex();
  friend void reset_random_state();

private:
  std::mutex mtx;

  std::mt19937_64 mt64;

  std::uniform_int_distribution<uint64_t> distribution;
};

uint8_t random8();

uint16_t random16();

uint32_t random32();

uint64_t random64();

double random_decimal();

uint64_t random_range(uint64_t begin, uint64_t end);

#ifndef __ORS_COMMON_RAND__
#define __ORS_COMMON_RAND__
template <typename T> T random_range(T begin, T end) {
  return T(lround(begin + random_decimal() * (end - begin)));
}

template <> inline double random_range(double begin, double end) {
  return begin + random_decimal() * (end - begin);
}

template <> inline float random_range(float begin, float end) {
  return begin + random_decimal() * (end - begin);
}
#endif
} // namespace rand
} // namespace utils
} // namespace ors

#endif