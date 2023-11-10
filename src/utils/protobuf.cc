#include <google/protobuf/text_format.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <utils/protobuf.h>
#include <utils/string_util.h>
#include <utils/numeric.h>
namespace ors {
namespace utils {
namespace protobuf {

using ors::utils::string_util::replace_all;
using ors::utils::numeric::down_cast;
void fromString(const std::string &str, Message &protoBuf) {
  google::protobuf::LogSilencer lser; // 禁止protobuf库输出日志信息
  google::protobuf::TextFormat::ParseFromString(str, &protoBuf);
}

std::string dump_string(const google::protobuf::Message &msg,
                       bool forCopyingIntoTest) {
  std::string ret;
  google::protobuf::TextFormat::Printer printer;
  if (forCopyingIntoTest) {
    // Most lines that use these strings will look like this:
    // ^    EXPECT_EQ(...,$
    // ^              "..."
    // ^              "...");
    //  12345678901234
    // Therefore, we want 14 leading spaces. Tell gtest we want 16, though,
    // so that when we add in the surrounding quotes later, lines won't
    // wrap.
    printer.SetInitialIndentLevel(8);
  }
  printer.SetUseShortRepeatedPrimitives(true);
  printer.PrintToString(msg, &ret);
  if (forCopyingIntoTest) {
    // TextFormat::Printer escapes ' already.
    replace_all(ret, "\"", "'");
    replace_all(ret, "                ", "              \"");
    replace_all(ret, "\n", "\"\n");
  }
  if (!msg.IsInitialized()) {
    std::vector<std::string> errors;
    msg.FindInitializationErrors(&errors);
    std::ostringstream os;
    os << ret;
    for (auto it = errors.begin(); it != errors.end(); ++it) {
      if (forCopyingIntoTest) {
        os << "              \"" << *it << ": UNDEFINED\"\n";
      } else {
        os << *it << ": UNDEFINED\n";
      }
    }
    return ret + os.str();
  }
  return ret;
}

std::unique_ptr<Message> copy(const Message &msg) {
  std::unique_ptr<Message> ret(msg.New());
  ret->CopyFrom(msg);
  return ret;
}

bool parse(const ors::utils::buffer &from, Message &to, size_t skip_bytes) {
  google::protobuf::LogSilencer lser;
  if (!to.ParseFromArray(static_cast<const char *>(from.data()) + skip_bytes,
                         down_cast<int>(from.length() - skip_bytes))) {
    spdlog::error("Missing fields in protocol buffer of type %s: %s",
                  to.GetTypeName().c_str(),
                  to.InitializationErrorString().c_str());
    return false;
  }
  return true;
}

void serialize(const Message &from, ors::utils::buffer &to, size_t skip_bytes) {
  // SerializeToArray seems to always return true, so we explicitly check
  // IsInitialized to make sure all required fields are set.
  if (!from.IsInitialized()) {
    spdlog::error("Missing fields in protocol buffer of type %s: %s (have %s)",
                  from.GetTypeName().c_str(),
                  from.InitializationErrorString().c_str(),
                  dump_string(from).c_str());
  }
  size_t length = size_t(from.ByteSize());
  char *data = new char[skip_bytes + length];
  from.SerializeToArray(data + skip_bytes, int(length));
  to.set_data(data, skip_bytes + length,
             ors::utils::buffer::del_array_func<char>);
}

} // namespace protobuf
} // namespace utils
} // namespace ors

namespace google {
namespace protobuf {

bool operator==(const Message &a, const Message &b) {
  // This is a close enough approximation of equality.
  return (a.GetTypeName() == b.GetTypeName() &&
          a.DebugString() == b.DebugString());
}

bool operator!=(const Message &a, const Message &b) { return !(a == b); }

bool operator==(const Message &a, const std::string &bStr) {
  std::unique_ptr<Message> b(a.New());
  LogSilencer lser;
  TextFormat::ParseFromString(bStr, b.get());
  return (a == *b);
}

bool operator==(const std::string &a, const Message &b) { return (b == a); }

bool operator!=(const Message &a, const std::string &b) { return !(a == b); }

bool operator!=(const std::string &a, const Message &b) { return !(a == b); }

} // namespace protobuf
} // namespace google