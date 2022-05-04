#include <util/helpers.hpp>

#include <yaclib/fault/config.hpp>
#include <yaclib/log.hpp>

#include <cstdio>

#include <gtest/gtest.h>

namespace test {

void InitLog() noexcept {
  auto assert_callback = [](std::string_view file, std::size_t line, std::string_view /*function*/,
                            std::string_view /*condition*/, std::string_view message) {
    GTEST_MESSAGE_AT_(file.data(), line, message.data(), ::testing::TestPartResult::kFatalFailure);
  };
  YACLIB_INIT_ERROR(assert_callback);
  YACLIB_INIT_INFO(nullptr);
  YACLIB_INIT_DEBUG(assert_callback);
}

void InitFault() {
  yaclib::SetFaultFrequency(8);
  yaclib::SetFaultSleepTime(200);
}

namespace {

void PrintEnvInfo() {
#ifdef __GLIBCPP__
  std::fprintf(stderr, "libstdc++: %d\n", __GLIBCPP__);
#endif
#ifdef __GLIBCXX__
  std::fprintf(stderr, "libstdc++: %d\n", __GLIBCXX__);
#endif
#ifdef _LIBCPP_VERSION
  std::fprintf(stderr, "libc++: %d\n", _LIBCPP_VERSION);
#endif
}

}  // namespace
}  // namespace test

int main(int argc, char** argv) {
  test::PrintEnvInfo();
  test::InitLog();
  test::InitFault();
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
