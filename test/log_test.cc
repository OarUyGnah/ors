#include "spdlog/spdlog.h"

void test_spdlog() {
  spdlog::set_level(spdlog::level::debug);
  spdlog::debug("Backtrace message {}", 33);
  spdlog::info("set spdlog {}", "dependencies ok");
}

int main(int argc, char const *argv[]) {
  test_spdlog();
  return 0;
}
