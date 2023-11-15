#ifndef __ORS_UTILS_STRINGUTIL_H__
#define __ORS_UTILS_STRINGUTIL_H__
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
namespace ors {
namespace utils {
namespace string {
bool printable(const char *str);

bool printable(const void *data, size_t length);

std::string fmt(const char *format, ...) __attribute__((format(printf, 1, 2)));

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

namespace stl {
// random access iterator sort
template <typename T> void sort(T &container) {
  std::sort(container.begin(), container.end());
}

template <template <typename...> class Vec = std::vector, typename MapLike>
auto keys(const MapLike &map) -> Vec<typename MapLike::key_type> {
  Vec<typename MapLike::key_type> keys;
  for (auto it = map.begin(); it != map.end(); ++it)
    keys.push_back(it->first);
  return keys;
}

template <template <typename...> class Vec = std::vector, typename MapLike>
auto values(const MapLike &map) -> Vec<typename MapLike::mapped_type> {
  Vec<typename MapLike::mapped_type> values;
  for (auto it = map.begin(); it != map.end(); ++it)
    values.push_back(it->second);
  return values;
}

template <template <typename...> class Vec = std::vector, typename MapLike>
auto pairs(const MapLike &map) -> Vec<typename MapLike::value_type> {
  std::vector<typename MapLike::value_type> items;
  for (auto it = map.begin(); it != map.end(); ++it)
    items.push_back(*it);
  return items;
}
} // namespace stl

} // namespace utils
} // namespace ors

#endif // !__ORS_UTILS_STRINGUTIL_H__