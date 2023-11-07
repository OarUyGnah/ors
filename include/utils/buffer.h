#ifndef __ORS_UTILS_BUFFER_H__
#define __ORS_UTILS_BUFFER_H__

#include <cinttypes>
#include <cstdlib>
#include <utils/noncopyable.h>
namespace ors {
namespace utils {
class buffer : public noncopyable {
public:
  using deleter = void (*)(void *data);
  // typedef void (*deleter)(void *data);

public:
  buffer();

  buffer(void *data, uint64_t length, deleter del_fn);

  buffer(buffer &&);

  buffer &operator=(buffer &&);

  ~buffer();

  void *data();

  const void *data() const;

  size_t length();

  size_t length() const;

  void set_data(void *data, size_t length, deleter del_fn);

  void clear();

  template <typename T> static void del_object_func(void *data) {
    delete static_cast<T>(data);
  }

  template <typename T> static void del_array_func(void *data) {
    delete[] static_cast<T *>(data);
  }

private:
  void *buf;

  size_t len;

  deleter del_fn;
};
} // namespace utils
} // namespace ors

#endif // !__ORS_UTILS_BUFFER_H__