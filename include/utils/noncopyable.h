#ifndef __ORS_UTILS_NONCOPYABLE_H__
#define __ORS_UTILS_NONCOPYABLE_H__
namespace ors {
namespace utils {

class noncopyable {
protected:
  noncopyable() = default;

  ~noncopyable() = default;

private:
  noncopyable(const noncopyable &) = delete;

  noncopyable &operator=(const noncopyable &) = delete;
};

} // namespace core
} // namespace ors

#endif // !__ORS_UTILS_NONCOPYABLE_H__