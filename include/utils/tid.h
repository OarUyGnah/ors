#ifndef __ORS_UTILS_TID_H__
#define __ORS_UTILS_TID_H__

#include <cinttypes>
#include <exception>
#include <string>
namespace ors {
namespace utils {
namespace tid {

struct thread_name_alredy_exist : public std::exception {
  explicit thread_name_alredy_exist(const std::string &name)
      : thread_name(name) {}
  virtual ~thread_name_alredy_exist() throw() {}
  std::string thread_name;
};

uint64_t tid();

void set_name(std::string name);

std::string name();

} // namespace tid
} // namespace utils
} // namespace ors

#endif // !__ORS_UTILS_TID_H__