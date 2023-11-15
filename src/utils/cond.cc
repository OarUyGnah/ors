#include <spdlog/spdlog.h>
#include <string.h>
#include <utils/cond.h>

namespace ors {
namespace utils {
condition_variable::condition_variable()
    : cv(), cb(), notify_count(0), last_wu() {
  init();
}

condition_variable::~condition_variable() {
  int ret = pthread_cond_destroy(&cv);
  if (ret != 0)
    spdlog::error("pthread_cond_destroy : %s", strerror(ret));
}

void condition_variable::notify(notify_type t) {
  switch (t) {
  case ONE:
    notify_one();
    break;
  case ALL:
    notify_all();
    break;
  default:
    break;
  }
}

void condition_variable::notify_one() {
  ++notify_count;
  int ret = pthread_cond_signal(&cv);
  if (ret != 0)
    spdlog::error("pthread_cond_signal : %s", strerror(ret));
}

void condition_variable::notify_all() {
  ++notify_count;
  int ret = pthread_cond_broadcast(&cv);
  if (ret != 0)
    spdlog::error("pthread_cond_broadcast : %s", strerror(ret));
}

void condition_variable::wait(std::unique_lock<std::mutex> &ul) {
  if (cb) {
    ul.unlock();
    cb();
    ul.lock();
  } else {
    pthread_mutex_t *mutex = ul.mutex()->native_handle();
    int ret = pthread_cond_wait(&cv, mutex);
    if (ret != 0)
      spdlog::error("pthread_cond_wait : %s", strerror(ret));
  }
}

void condition_variable::wait(std::unique_lock<utils::mutex> &ul) {
  utils::mutex &mutex(*ul.mutex());
  if (mutex.cb)
    mutex.cb();
  assert(ul);
  std::unique_lock<std::mutex> stdLockGuard(mutex.mtx, std::adopt_lock_t());
  ul.release();
  wait(stdLockGuard);
  assert(stdLockGuard);
  ul = std::unique_lock<utils::mutex>(mutex, std::adopt_lock_t());
  stdLockGuard.release();
  if (mutex.cb)
    mutex.cb();
}

void condition_variable::wait_until(
    std::unique_lock<std::mutex> &ul,
    const utils::time::steady_clock::time_point &abs_tp) {
  last_wu = abs_tp;
  if (cb) {
    ul.unlock();
    cb();
    ul.lock();
  } else {
    utils::time::steady_clock::time_point now =
        utils::time::steady_clock::now();
    utils::time::steady_clock::time_point wake =
        std::min(abs_tp, now + std::chrono::hours(1));
    if (wake < now)
      return;
    struct timespec wakespec = utils::time::make_timespec(wake);
    pthread_mutex_t *mutex = ul.mutex()->native_handle();
    int ret = pthread_cond_timedwait(&cv, mutex, &wakespec);
    switch (ret) {
    case 0:
      break;
    case ETIMEDOUT:
      break;
    default:
      spdlog::error("pthread_cond_timedwait : %s", strerror(ret));
    }
  }
}

void condition_variable::init() {
  pthread_condattr_t cvattr;
  int ret = pthread_condattr_init(&cvattr);
  if (ret != 0)
    spdlog::error("pthread_condattr_init : %s", strerror(ret));
  ret = pthread_condattr_setclock(&cvattr, utils::time::STEADY_CLOCK_ID);
  if (ret != 0)
    spdlog::error("pthread_condattr_setclock : %s", strerror(ret));
  ret = pthread_cond_init(&cv, &cvattr);
  if (ret != 0)
    spdlog::error("pthread_cond_init : %s", strerror(ret));
  ret = pthread_condattr_destroy(&cvattr);
  if (ret != 0)
    spdlog::error("pthread_condattr_destroy : %s", strerror(ret));
}
} // namespace utils
} // namespace ors