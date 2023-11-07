#ifndef __ORS_UTILS_CONFIG_H__
#define __ORS_UTILS_CONFIG_H__

#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>

namespace ors {
namespace utils {

class config {
public:
  using string = std::string;

  explicit config(const string &delimiter = "=", const string &comment = "#");

  explicit config(const std::map<string, string> &options);

  void read_file(const string &filename);

  friend std::istream &operator>>(std::istream &is, config &cf);

  friend std::ostream &operator<<(std::ostream &os, const config &cf);

  template <class T = string> T read(const string &key) const;

  template <class T = string> T read(const string &key, const T &value) const;

  bool has_key(const string &key) const;

  bool has_key(const string &key);

  template <class T> void set(const string &key, const T &value);

  void set(const string &key, const string &value);

  void remove(const string &key);

  string delim() const { return delimiter; }

  string delim() { return delimiter; }

  string comm() const { return comment; }

  string comm() { return comment; }

  std::string &operator[](string &key);

  std::string &&operator[](string &&key);

  // config exception
  struct exception : public std::runtime_error {
    explicit exception(const std::string &error);
  };

  struct file_not_found : public exception {
    explicit file_not_found(const string &filename);
    virtual ~file_not_found() throw() {}
    string filename;
  };

  // thrown only by T read(key) variant of read()
  struct key_not_found : public exception {
    explicit key_not_found(const string &key);
    virtual ~key_not_found() throw() {}
    string key;
  };

  struct conversion_error : public exception {
    conversion_error(const string &key, const string &value,
                     const string &typeName);
    virtual ~conversion_error() throw() {}
    string key;
    string value;
    string typeName;
  };

  // private:
public:
  template <class T> static string to_string(const T &t);

  template <class T> static T from_string(const string &key, const string &s);

  std::string read_line(std::istream &is) const;

  const string delimiter;

  const string comment;

  std::map<string, string> contents;
};

template <class T> std::string config::to_string(const T &t) {
  std::ostringstream ost;
  ost.setf(std::ostringstream::boolalpha);
  ost << t;
  return ost.str();
}

template <class T> T config::from_string(const string &key, const string &s) {
  T t;
  std::istringstream ist(s);
  ist >> t;
  if (!ist || !ist.eof())
    throw conversion_error(key, s, typeid(T).name());
  return t;
}

template <>
std::string config::from_string<std::string>(const string &key,
                                             const string &s);

template <> bool config::from_string<bool>(const string &key, const string &s);

template <class T> T config::read(const string &key) const {
  auto p = contents.find(key);
  if (p == contents.end())
    throw key_not_found(key);
  return from_string<T>(key, p->second);
}

template <class T> T config::read(const string &key, const T &value) const {
  auto p = contents.find(key);
  if (p == contents.end())
    return value;
  return from_string<T>(key, p->second);
}

template <class T> void config::set(const string &key, const T &value) {
  set(key, to_string(value));
}

} // namespace utils
} // namespace ors

#endif // !__ORS_UTILS_CONFIG_H__