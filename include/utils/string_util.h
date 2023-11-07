#ifndef __ORS_UTILS_STRINGUTIL_H__
#define __ORS_UTILS_STRINGUTIL_H__
#include <sstream>
#include <string>
#include <vector>
namespace ors {
namespace utils {
namespace string_util {
bool printable(const char *str);

bool printable(const void *data, size_t length);

std::string fmt(const char *format, ...)
    __attribute__((format(printf, 1, 2)));

std::string flags(int value,
                  std::initializer_list<std::pair<int, const char *>> flags);

std::string trim(const std::string &s);

std::string join(const std::vector<std::string> &components,
                 const std::string &glue);

std::vector<std::string> split(const std::string &subject, char delimiter);

void replace_all(std::string &haystack, const std::string &needle,
                 const std::string &replacement);

bool starts_with(const std::string &haystack, const std::string &needle);

bool ends_with(const std::string &haystack, const std::string &needle);

template <typename T> std::string to_string(const T &t) {
  std::stringstream ss;
  ss << t;
  return std::move(ss.str());
}

} // namespace string
} // namespace utils
} // namespace ors

#endif // !__ORS_UTILS_STRINGUTIL_H__