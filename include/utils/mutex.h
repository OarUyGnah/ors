#ifndef __ORS_MUTEX_H__
#define __ORS_MUTEX_H__

#include <cassert>
#include <functional>
#include <mutex>
namespace ors {
namespace utils {
class mutex {
public:
// 
  mutex() : m(), cb() {}

  void lock() {
    m.lock();
    if (cb)
      cb();
  }

  bool try_lock() {
    bool l = m.try_lock();
    if (l) {
      if (cb)
        cb();
    }
    return l;
  }

  void unlock() {
    // TODO(ongardie): apparently try_lock->false...unlock is allowed, but
    // this will then call the callback without the lock, which is unsafe.
    if (cb)
      cb();
    m.unlock();
  }
    // HANDLE(windows) or pthread_mutex_t(Linux)
  std::mutex::native_handle_type native_handle() { return m.native_handle(); }

private:
  /// Underlying mutex.
  ::std::mutex m;

public:
  /**
   * This function will be called with the lock held after the lock is
   * acquired and before it is released.
   */
  std::function<void()> cb;

  friend class condition_variable;
};

/**
 * Release a mutex upon construction, reacquires it upon destruction.
 * \tparam mutex
 *      Type of mutex (either std::mutex or Core::mutex).
 */
template <typename mutex> class mutex_unlock {
public:
  explicit mutex_unlock(std::unique_lock<mutex> &guard) : guard(guard) {
    assert(guard.owns_lock());
    guard.unlock();
  }
  ~mutex_unlock() { guard.lock(); }

private:
  std::unique_lock<mutex> &guard;
};

/**
 * Proof that the caller is holding some mutex.
 * Useful as an additional (unused) argument for some private methods that want
 * to ensure the caller is holding a lock.
 */
class holding_mutex {
public:
  /**
   * Constructor from std::lock_guard.
   */
  template <typename mutex>
  explicit holding_mutex(const std::lock_guard<mutex> &lg) {}

  /**
   * Constructor from std::unique_lock. Since unique_lock might not, in fact,
   * hold the lock, this uses a dynamic check in the form of an assert().
   */
  template <typename mutex>
  explicit holding_mutex(const std::unique_lock<mutex> &lg) {
    assert(lg.owns_lock());
  }
};
} // namespace utils
} // namespace ors

#endif // !__ORS_MUTEX_H__
