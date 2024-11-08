#include <y/error.h>
#include "details.h"
#include "throwRuntimeError.h"
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <thread>
#include <semaphore>

using namespace testing;
using namespace y::error;

TEST(Tests, BasicStackCapture)
{
    y::error::Context errorContext;
    ASSERT_THROW(throwRuntimeError(), std::runtime_error);

    std::ostringstream s;
    s << errorContext.getStackTrace();
    EXPECT_THAT(s.str(), HasSubstr("  throwRuntimeError()"));
}

TEST(Tests, ContextCapture)
{
    y::error::Context errorContext;
    ASSERT_THROW(throwWithDetail<SomeDetail>("something"), std::runtime_error);
    ASSERT_TRUE(errorContext.hasDetail<SomeDetail>());
    ASSERT_EQ(errorContext.getDetail<SomeDetail>(), "something");

    std::ostringstream s;
    s << errorContext.getDetails();
    EXPECT_THAT(s.str(), HasSubstr("  SomeDetail: something"));
}

TEST(Tests, Helper)
{
    auto result = y::error::handleExceptionsWithContext(
            [] {
                throwWithDetail<SomeDetail>("something");
                return std::string("wrong"); },
            [](const y::error::Context& context, std::exception_ptr e) {
                std::ostringstream s;
                try { std::rethrow_exception(e); }
                catch (const std::exception& e) { s << "Exception caught:\n  " << e << '\n'; }
                catch(...) {}
                s << context;
                return s.str(); });
    EXPECT_THAT(result, HasSubstr("  std::runtime_error: Runtime error description"));
    EXPECT_THAT(result, HasSubstr("  SomeDetail: something"));
    EXPECT_THAT(result, HasSubstr("  throwRuntimeError()"));
}

TEST(Tests, HelperWithVoidFunction)
{
    ASSERT_THROW(
            y::error::handleExceptionsWithContext(
                [] { throwWithDetail<SomeDetail>("something"); },
                [](const y::error::Context& context, std::exception_ptr e) { std::rethrow_exception(e); }),
            std::runtime_error);
}

TEST(Tests, MainHandler)
{
  std::stringstream ss;
  auto result = y::error::handleExceptionsWithContext(
      []()->int{ throwRuntimeError(); },
      makeMainPrintErrorHandler(ss, 25));
  ASSERT_EQ(result, 25);
  EXPECT_THAT(ss.str(), HasSubstr("  throwRuntimeError()"));
}

TEST(Tests, ThreadHandler)
{
  using namespace std::chrono_literals;
  std::binary_semaphore sem{0};
  std::stringstream ss;
  std::thread t([&]{
      y::error::handleExceptionsWithContext(
        [&](){
          sem.release();
          std::this_thread::sleep_for(10s); },
        makeThreadPrintErrorHandler(ss));
  });
  sem.acquire();
  pthread_cancel(t.native_handle());
  t.join();
  EXPECT_EQ(ss.str(), "");
}