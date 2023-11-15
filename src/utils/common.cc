#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdarg>
#include <cstring>
#include <functional>
#include <locale>
#include <utils/common.h>

namespace ors {
namespace utils {
namespace string {
bool displayable(char c) { return c >= 32 && c < 127; }

bool printable(const char *str) { return printable(str, strlen(str) + 1); }

bool printable(const void *data, size_t length) {
  const char *begin = static_cast<const char *>(data);
  const char *end = begin + length - 1;
  return (length >= 1 && *end == '\0' && std::all_of(begin, end, displayable));
}

std::string fmt(const char *format, ...) {
  std::string s;
  va_list ap;
  va_start(ap, format);

  // We're not really sure how big of a buffer will be necessary.
  // Try 1K, if not the return value will tell us how much is necessary.
  size_t buf_size = 1024;
  while (true) {
    char buf[buf_size];
    // vsnprintf trashes the va_list, so copy it first
    va_list aq;
    va_copy(aq, ap);
    int r = vsnprintf(buf, buf_size, format, aq);
    va_end(aq);
    assert(r >= 0); // old glibc versions returned -1
    size_t r2 = size_t(r);
    if (r2 < buf_size) {
      s = buf;
      break;
    }
    buf_size = r2 + 1;
  }

  va_end(ap);
  return s;
}

std::string flags(int value,
                  std::initializer_list<std::pair<int, const char *>> flags) {
  if (value == 0)
    return "0";
  std::vector<std::string> strings;
  for (auto it = flags.begin(); it != flags.end(); ++it) {
    if (value & it->first) {
      strings.push_back(it->second);
      value &= ~it->first;
    }
  }
  if (value)
    strings.push_back(fmt("0x%x", value));
  return join(strings, "|");
}

std::string trim(const std::string &original) {
  std::string s = original;
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       std::not1(std::ptr_fun<int, int>(std::isspace)))
              .base(),
          s.end());
  s.erase(s.begin(),
          std::find_if(s.begin(), s.end(),
                       std::not1(std::ptr_fun<int, int>(std::isspace))));
  return std::move(s);
}

std::string join(const std::vector<std::string> &components,
                 const std::string &glue) {
  std::string r;
  for (size_t i = 0; i < components.size(); ++i) {
    r += components.at(i);
    if (i < components.size() - 1)
      r += glue;
  }
  return std::move(r);
}

std::vector<std::string> split(const std::string &subject, char delimiter) {
  std::vector<std::string> items;
  std::istringstream stream(subject);
  std::string item;
  while (std::getline(stream, item, delimiter))
    items.push_back(std::move(item));
  return std::move(items);
}

void replace_all(std::string &haystack, const std::string &needle,
                 const std::string &replacement) {
  size_t startPos = 0;
  while (true) {
    size_t replacePos = haystack.find(needle, startPos);
    if (replacePos == haystack.npos)
      return;
    haystack.replace(replacePos, needle.length(), replacement);
    startPos = replacePos + replacement.length();
  }
}

bool starts_with(const std::string &haystack, const std::string &needle) {
  return (haystack.compare(0, needle.length(), needle) == 0);
}

bool ends_with(const std::string &haystack, const std::string &needle) {
  if (haystack.length() < needle.length())
    return false;
  return (haystack.compare(haystack.length() - needle.length(), needle.length(),
                           needle) == 0);
}

} // namespace string

namespace stl {

}

} // namespace utils
} // namespace ors
