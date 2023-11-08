#ifndef __ORS_UTILS_PROTOBUF_H__
#define __ORS_UTILS_PROTOBUF_H__

#include <google/protobuf/message.h>
#include <memory>
#include <string>
#include <utils/buffer.h>

namespace ors {
namespace utils {
namespace protobuf {
using namespace ::google::protobuf;
void from_string(const std::string &str, Message &msg);

template <typename T = Message> T from_string(const std::string &str) {
  T t;
  fromString(str, t);
  return t;
}

std::string dump_string(const Message &msg, bool forCopyingIntoTest = false);

std::unique_ptr<Message> copy(const Message &msg);

bool parse(const ors::utils::buffer &from, Message &to,
           size_t skip_bytes = 0);

void serialize(const Message &from, ors::utils::buffer &to,
               size_t skip_bytes = 0);

struct istream {
  virtual ~istream() = default;
  virtual size_t get_bytes_read() const = 0;
  virtual std::string readMessage(google::protobuf::Message &msg) = 0;
  virtual size_t readRaw(void *data, size_t length) = 0;
};

struct ostream {
  virtual ~ostream() = default;
  virtual size_t get_bytes_writen() const = 0;
  virtual void write_message(const google::protobuf::Message &msg) = 0;
  virtual void write_raw(const void *data, size_t length) = 0;
};

} // namespace protobuf
} // namespace utils
} // namespace ors

namespace google {
namespace protobuf {

/**
 * Equality for protocol buffers so that they can be used in EXPECT_EQ.
 * This is useful for testing.
 */
bool operator==(const Message &a, const Message &b);

/**
 * Inequality for protocol buffers so that they can be used in EXPECT_NE.
 * This is useful for testing.
 */
bool operator!=(const Message &a, const Message &b);

// Equality and inequality between protocol buffers and their text format
// representations. These are useful for testing.
bool operator==(const Message &a, const std::string &b);
bool operator==(const std::string &a, const Message &b);
bool operator!=(const Message &a, const std::string &b);
bool operator!=(const std::string &a, const Message &b);

} // namespace protobuf
} // namespace google
#endif // !__ORS_UTILS_PROTOBUF_H__