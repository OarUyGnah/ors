#include <fcntl.h>
#include <filesystem>
#include <gtest/gtest.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utils/config.h>
using namespace ors::utils;
// TEST(config_test, 1) {
//   config c(",", "#");
//   c["123"] = "123";
//   auto val = c["123"];
//   EXPECT_EQ(val, "123");
//   // extern void __lrtrim(std::string& s);
//   // std::string s(" 21312312 ");
//   // __lrtrim(s);
// }

using std::map;
using std::string;
static char *path = "/tmp/ors_configtest";
class CoreConfigTest : public ::testing::Test {
public:
  CoreConfigTest() : tmpdir(path) {
    // std::cerr << "tmpdir == " << tmpdir << std::endl;
    string str = tmpdir + "/test.txt";
    // std::cerr << "path == " << str << std::endl;
    // 要保证对应的路径文件夹都已经创建
    std::filesystem::path p = "/tmp/ors_configtest/";
    std::filesystem::create_directory(p);
    int fd = open(str.c_str(), O_WRONLY | O_CREAT, 0644);
    if (fd < 0) {
      std::cerr << "Couldn't open temp file" << std::endl;
    }
    const char *file = "# this is a sample config file \n"
                       "        # here's a comment in a random place \n"
                       " double = 3.14  \n"
                       " int = 314  #  comment  \n"
                       " string = a = b = c = d  #  newline comes next  \n"
                       "\n"
                       " multiline = \n"
                       "                strange\n"
                       "# just a comment\n"
                       " empty =  \n";
    if (write(fd, file, uint32_t(strlen(file))) == -1)
      std::cerr << "Couldn't write to temp file" << std::endl;
    close(fd);
  }
  ~CoreConfigTest() {
    try {
      std::filesystem::remove_all(tmpdir); 
      std::cout << "remove test dir successed" << std::endl;
    } catch (const std::filesystem::filesystem_error &e) {
      std::cerr << "remove test dir failed" << e.what() << std::endl;
    }
  }
  std::string tmpdir;
};

TEST_F(CoreConfigTest, constructor_delim) {
  config cfg;
  EXPECT_EQ("=", cfg.delimiter);
  EXPECT_EQ("#", cfg.comment);
  EXPECT_TRUE(cfg.contents.empty());

  config cfg2(":", "//");
  EXPECT_EQ(":", cfg2.delimiter);
  EXPECT_EQ("//", cfg2.comment);
  EXPECT_TRUE(cfg2.contents.empty());
}

TEST_F(CoreConfigTest, constructor_withOptions) {
  map<string, string> options = {
      {"double", "3.14"}, {"int", "314"}, {"string", "a = b = c = d"}};
  config config(options);
  EXPECT_EQ(options, config.contents);
}

TEST_F(CoreConfigTest, readFile) {
  config config;
  config.read_file(tmpdir + "/test.txt");
  EXPECT_THROW(config.read_file(tmpdir + "/bogus"), config::file_not_found);
}

TEST_F(CoreConfigTest, streamIn) {
  config config;
  config.read_file(tmpdir + "/test.txt");
  EXPECT_EQ((map<string, string>{
                {"double", "3.14"},
                {"int", "314"},
                {"string", "a = b = c = d"},
                {"multiline", "\nstrange"},
                {"empty", ""},
            }),
            config.contents);
}

TEST_F(CoreConfigTest, streamOut) {
  config config;
  config.set("k", "v");
  config.set("x", "z");
  std::stringstream ss;
  ss << config;
  EXPECT_EQ("k = v\nx = z\n", ss.str());
}

TEST_F(CoreConfigTest, has_key) {
  config config;
  config.set("k", "v");
  EXPECT_TRUE(config.has_key("k"));
  EXPECT_FALSE(config.has_key("bogus"));
}

TEST_F(CoreConfigTest, readWithoutDefault) {
  config config;
  config.set("k", "v");
  EXPECT_THROW(config.read("bogus"), config::key_not_found);
  EXPECT_EQ("v", config.read("k"));
  EXPECT_THROW(config.read<int>("k"), config::conversion_error);
}

TEST_F(CoreConfigTest, readWithDefault) {
  config config;
  config.set("k", "v");
  EXPECT_EQ("foo", config.read<string>("bogus", "foo"));
  EXPECT_EQ("v", config.read<string>("k", "bar"));
  EXPECT_THROW(config.read("k", 7), config::conversion_error);
}

TEST_F(CoreConfigTest, set) {
  config config;
  config.set("  k  ", "  v  ");
  EXPECT_EQ((map<string, string>{
                {"k", "v"},
            }),
            config.contents);
}

TEST_F(CoreConfigTest, remove) {
  config config;
  config.set("k", "v");
  config.set("k2", "v2");
  config.remove("k");
  config.remove("bogus");
  EXPECT_EQ((map<string, string>{
                {"k2", "v2"},
            }),
            config.contents);
}

TEST_F(CoreConfigTest, toString) {
  EXPECT_EQ("1", config::to_string(1));
  EXPECT_EQ("1", config::to_string(1U));
  EXPECT_EQ("18446744073709551615", config::to_string(~0UL));
  EXPECT_EQ("3.14", config::to_string(3.14));
  EXPECT_EQ("-9", config::to_string(-9));
  EXPECT_EQ("pie", config::to_string("pie"));
  EXPECT_EQ("0", config::to_string(0));
  EXPECT_EQ("false", config::to_string(false));
  EXPECT_EQ("true", config::to_string(true));
}

TEST_F(CoreConfigTest, fromString) {
  EXPECT_EQ("foo", config::from_string<string>("k", "foo"));
  EXPECT_EQ(-1, config::from_string<int>("k", "-1"));
  EXPECT_EQ(0, config::from_string<int>("k", "0"));
  EXPECT_EQ(1, config::from_string<int>("k", "1"));
  EXPECT_EQ(~0U, config::from_string<uint32_t>("k", "4294967295"));
  EXPECT_EQ(5.3, config::from_string<double>("k", "5.3"));
  EXPECT_THROW(config::from_string<int>("k", ""), config::conversion_error);
  EXPECT_THROW(config::from_string<int>("k", "a"), config::conversion_error);
  EXPECT_THROW(config::from_string<int>("k", "5a"), config::conversion_error);
  EXPECT_THROW(config::from_string<int>("k", "5.3"), config::conversion_error);
  EXPECT_THROW(config::from_string<double>("k", "a"), config::conversion_error);
  EXPECT_THROW(config::from_string<int>("k", "2147483648"),
               config::conversion_error);
  EXPECT_THROW(config::from_string<int>("k", "-2147483649"),
               config::conversion_error);
  EXPECT_THROW(config::from_string<uint32_t>("k", "4294967296"),
               config::conversion_error);
  EXPECT_THROW(config::from_string<int>("k", "0xFFFFFFFF"),
               config::conversion_error);
  try {
    config::from_string<int>("k", "1.5");
  } catch (const config::conversion_error &e) {
    EXPECT_STREQ("The value k for key 1.5 could not be converted to a int",
                 e.what());
  }

  EXPECT_FALSE(config::from_string<bool>("k", "false"));
  EXPECT_FALSE(config::from_string<bool>("k", "f"));
  EXPECT_FALSE(config::from_string<bool>("k", "no"));
  EXPECT_FALSE(config::from_string<bool>("k", "n"));
  EXPECT_FALSE(config::from_string<bool>("k", "FALSE"));
  EXPECT_FALSE(config::from_string<bool>("k", "F"));
  EXPECT_FALSE(config::from_string<bool>("k", "NO"));
  EXPECT_FALSE(config::from_string<bool>("k", "N"));
  EXPECT_FALSE(config::from_string<bool>("k", "0"));
  EXPECT_TRUE(config::from_string<bool>("k", "true"));
  EXPECT_TRUE(config::from_string<bool>("k", "t"));
  EXPECT_TRUE(config::from_string<bool>("k", "yes"));
  EXPECT_TRUE(config::from_string<bool>("k", "y"));
  EXPECT_TRUE(config::from_string<bool>("k", "TRUE"));
  EXPECT_TRUE(config::from_string<bool>("k", "T"));
  EXPECT_TRUE(config::from_string<bool>("k", "YES"));
  EXPECT_TRUE(config::from_string<bool>("k", "Y"));
  EXPECT_TRUE(config::from_string<bool>("k", "1"));
  EXPECT_THROW(config::from_string<bool>("k", "-1"), config::conversion_error);
}

TEST_F(CoreConfigTest, readLine) {
  // This is tested sufficiently in streamIn
}