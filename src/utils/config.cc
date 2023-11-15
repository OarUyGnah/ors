#include <cxxabi.h>
#include <fstream>
#include <functional>
#include <utils/config.h>
#include <utils/common.h>
namespace ors {
namespace utils {
static constexpr char __white_space[] = " \n\t\v\r\f";
static const std::map<std::string, bool> values{
    // These must be in sorted ASCII order for binary search to work. All
    // alpha entries should appear once in lowercase and once in uppercase.
    {"0", false}, {"1", true},   {"F", false}, {"FALSE", false},
    {"N", false}, {"NO", false}, {"T", true},  {"TRUE", true},
    {"Y", true},  {"YES", true}, {"f", false}, {"false", false},
    {"n", false}, {"no", false}, {"t", true},  {"true", true},
    {"y", true},  {"yes", true},
};

static void __ltrim(std::string &s) {
  s.erase(0, s.find_first_not_of(__white_space));
}

static void __rtrim(std::string &s) {
  s.erase(s.find_last_not_of(__white_space) + 1);
}

static void __lrtrim(std::string &s) {
  __ltrim(s);
  __rtrim(s);
}

std::string demangle(const std::string &name) {
  char *result = abi::__cxa_demangle(name.c_str(), nullptr, nullptr, nullptr);
  if (result == nullptr)
    return name;
  std::string ret(result);
  free(result);
  return std::move(ret);
}

config::config(const string &delimiter, const string &comment)
    : delimiter(delimiter), comment(comment), contents() {}

config::config(const std::map<string, string> &options)
    : delimiter("="), comment("#"), contents(options) {}

void config::read_file(const string &filename) {
  std::ifstream in(filename.c_str());
  if (!in)
    throw file_not_found(filename);
  in >> (*this);
}

std::istream &operator>>(std::istream &is, config &cfg) {
  using string = std::string;
  // might need to read ahead to see where value ends
  string next_line;

  while (is || !next_line.empty()) {
    string line; // 每次一行
    if (next_line.empty())
      line = std::move(cfg.read_line(is));
    else
      line.swap(next_line);
    auto delim_pos = line.find(cfg.delimiter);
    if (delim_pos != string::npos) {
      // Extract the key from line
      string key = line.substr(0, delim_pos);
      __rtrim(key);

      // Extract the value from line
      string value;
      line.swap(value);
      value.erase(0, delim_pos + cfg.delimiter.length());
      __ltrim(value);

      // See if value continues on the next line
      // Stop at empty line, next line with a key, or end of stream
      while (is) {
        line = cfg.read_line(is);
        // Empty lines end multi-line values
        if (line.empty())
          break;
        // Lines with delimiters end multi-line values
        delim_pos = line.find(cfg.delimiter);
        if (delim_pos != string::npos) {
          next_line.swap(line);
          break;
        }

        // Append this line to the multi-line value.
        value += "\n";
        value += line;
      }

      // Store key and value
      cfg.contents[key] = value; // overwrites if key is repeated
    }
  }

  return is;
}

std::ostream &operator<<(std::ostream &os, const config &cf) {
  for (auto p = cf.contents.begin(); p != cf.contents.end(); ++p) {
    os << p->first << " " << cf.delimiter << " " << p->second << std::endl;
  }
  return os;
}

bool config::has_key(const string &key) const {
  return (contents.find(key) != contents.end());
}

bool config::has_key(const string &key) {
  return (contents.find(key) != contents.end());
}

void config::set(const string &key, const string &value) {
  string k = key;
  string v = value;
  __lrtrim(k);
  __lrtrim(v);
  contents[k] = v;
}

void config::remove(const string &key) {
  auto it = contents.find(key);
  if (it != contents.end())
    contents.erase(it);
}

std::string &config::operator[](string &key) { return std::ref(contents[key]); }

std::string &&config::operator[](string &&key) {
  return std::move(contents[key]);
}

template <>
auto config::from_string<std::string>(const string &key, const string &s)
    -> std::string {
  return s;
}

template <>
auto config::from_string<bool>(const string &key, const string &s) -> bool {

  auto it = values.find(s);
  if (it != values.end())
    return it->second;
  throw conversion_error(key, s, "bool");
}

std::string config::read_line(std::istream &is) const {
  string line;
  std::getline(is, line);
  size_t comment_pos = line.find(comment);
  if (comment_pos != string::npos)
    line.erase(comment_pos);
  __lrtrim(line);
  return std::move(line);
}

config::exception::exception(const std::string &error)
    : std::runtime_error(error) {}

config::file_not_found::file_not_found(const string &filename)
    : exception(utils::string::fmt("The config file %s could not be opened",
                                 filename.c_str())),
      filename(filename) {}

config::key_not_found::key_not_found(const string &key)
    : exception(utils::string::fmt("The configuration does not specify %s",
                                 key.c_str())),
      key(key) {}

config::conversion_error::conversion_error(const string &key,
                                           const string &value,
                                           const string &typeName)
    : exception(utils::string::fmt(
          "The value %s for key %s could not be converted to a %s", key.c_str(),
          value.c_str(), demangle(typeName).c_str())),
      key(key), value(value), typeName(typeName) {}

} // namespace utils
} // namespace ors
