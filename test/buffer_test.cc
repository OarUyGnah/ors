#include <gtest/gtest.h>
#include <utils/buffer.h>
using namespace ors::utils;
uint32_t deleterCount;

void deleterCounter(void *data) { ++deleterCount; }

class buffer_test : public ::testing::Test {
public:
  buffer_test() {
    deleterCount = 0;
    strncpy(buf, "foo", sizeof(buf));
  }
  char buf[4];
};

TEST_F(buffer_test, constructor_default) {
  buffer buffer;
  EXPECT_TRUE(NULL == buffer.data());
  EXPECT_EQ(0U, buffer.length());
}

TEST_F(buffer_test, constructor_withData) {
  {
    buffer buffer(buf, sizeof(buf), deleterCounter);
    EXPECT_EQ(buf, buffer.data());
    EXPECT_EQ(sizeof(buf), buffer.length());
  }
  EXPECT_EQ(1U, deleterCount);
}

TEST_F(buffer_test, constructor_move) {
  {
    buffer buffer1(buf, sizeof(buf), deleterCounter);
    buffer buffer2(std::move(buffer1));
    EXPECT_TRUE(NULL == buffer1.data());
    EXPECT_EQ(0U, buffer1.length());
    EXPECT_EQ(buf, buffer2.data());
    EXPECT_EQ(sizeof(buf), buffer2.length());
  }
  EXPECT_EQ(1U, deleterCount);
}

TEST_F(buffer_test, destructor) {
  {
    buffer buffer1(buf, sizeof(buf), deleterCounter);
    buffer buffer2(buf, sizeof(buf), NULL);
  }
  EXPECT_EQ(1U, deleterCount);
}

TEST_F(buffer_test, assignment_move) {
  {
    buffer buffer1(buf, sizeof(buf), deleterCounter);
    buffer buffer2(buf + 1, sizeof(buf) - 1, deleterCounter);
    buffer2 = std::move(buffer1);
    EXPECT_TRUE(NULL == buffer1.data());
    EXPECT_EQ(0U, buffer1.length());
    EXPECT_EQ(buf, buffer2.data());
    EXPECT_EQ(sizeof(buf), buffer2.length());
  }
  EXPECT_EQ(2U, deleterCount);
}

TEST_F(buffer_test, getters_nonconst) {
  buffer buffer(buf, sizeof(buf), deleterCounter);
  EXPECT_EQ(buf, buffer.data());
  EXPECT_EQ(sizeof(buf), buffer.length());
}

TEST_F(buffer_test, getters_const) {
  const buffer buffer(buf, sizeof(buf), deleterCounter);
  EXPECT_EQ(buf, buffer.data());
  EXPECT_EQ(sizeof(buf), buffer.length());
}

TEST_F(buffer_test, setData) {
  {
    buffer buffer;
    buffer.set_data(buf, sizeof(buf), deleterCounter);
    EXPECT_EQ(buf, buffer.data());
    EXPECT_EQ(sizeof(buf), buffer.length());
    buffer.set_data(buf + 1, sizeof(buf) - 1, deleterCounter);
    EXPECT_EQ(buf + 1, buffer.data());
    EXPECT_EQ(sizeof(buf) - 1, buffer.length());
  }
  EXPECT_EQ(2U, deleterCount);
}

TEST_F(buffer_test, reset) {
  {
    buffer buffer;
    buffer.clear();
    buffer.set_data(buf, sizeof(buf), deleterCounter);
    buffer.clear();
    EXPECT_EQ(1U, deleterCount);
    EXPECT_TRUE(NULL == buffer.length());
    EXPECT_EQ(0U, buffer.length());
  }
  EXPECT_EQ(1U, deleterCount);
}