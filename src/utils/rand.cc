#include <cmath>
#include <cstring>
#include <fcntl.h>
#include <limits>
#include <mutex>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>

#include <utils/rand.h>
namespace ors {
namespace utils {
namespace rand {
static prng _prng;

void acquire_mutex() { _prng.mutex().lock(); }

/**
 * Called in the parent post fork(). This function is outside of the
 * anonymous namespace so it can be called from RandomTest.
 */
void release_mutex() { _prng.mutex().unlock(); }
void reset_random_state() {
  // we will have grabbed the mutex in pthread_atfork prepare, need
  // to release here
  release_mutex();
  _prng.reset_seed();
}

prng::prng() : mtx(), distribution(0, std::numeric_limits<uint64_t>::max()) {
  reset_seed();
  int err = pthread_atfork(acquire_mutex, release_mutex, reset_random_state);
  if (err != 0) {
    // too early to call ERROR in here
    fprintf(stderr,
            "Failed to set up pthread_atfork() handler to "
            "reset prng seed in child processes. "
            "As a result, child processes will generate the same "
            "sequence of random values as the parent they were forked "
            "from. Error: %s\n",
            strerror(err));
  }
}

prng::~prng() {}

void prng::reset_seed() {
  std::lock_guard<std::mutex> lockGuard(mtx);
  int fd = open("/dev/urandom", O_RDONLY);
  if (fd < 0) {
    // too early to call PANIC in here
    fprintf(stderr, "Open /dev/urandom failed: %s\n", strerror(errno));
    abort();
  }
  uint_fast64_t seed;
  ssize_t bytesRead = read(fd, &seed, sizeof(seed));
  close(fd);
  if (bytesRead != sizeof(seed)) {
    // too early to call PANIC in here
    fprintf(stderr, "Read seed error from /dev/urandom\n");
    abort();
  }
  mt64.seed(seed);
  static_assert(RAND_MAX >= (1L << 30), "RAND_MAX too small");
  // init = true;
}

uint64_t prng::random64() {
  auto r = distribution(mt64);
  return r;
}

std::mutex &prng::mutex() { return mtx; }

template <typename T> T random() { return T(_prng.random64()); }

uint8_t random8() { return random<uint8_t>(); }

uint16_t random16() { return random<uint16_t>(); }

uint32_t random32() { return random<uint32_t>(); }

uint64_t random64() { return _prng.random64(); }

double random_decimal() {
  return double(random64()) / double(std::numeric_limits<uint_fast64_t>::max());
}

uint64_t random_range(uint64_t begin, uint64_t end) {
  return begin + random_decimal() * (end - begin);
}

} // namespace rand
} // namespace utils
} // namespace ors