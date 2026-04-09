#include "details.h"
#include "throw-runtime-error.h"
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <semaphore>
#include <thread>
#include <y/error.h>

using namespace testing;
using namespace y::error;

TEST(Tests, BasicStackCapture) {
    Context errorContext;
    ASSERT_THROW(throwRuntimeError(), std::runtime_error);

    std::ostringstream s;
    s << errorContext.get_stack_trace();
    EXPECT_THAT(s.str(), HasSubstr("  throwRuntimeError()"));
}

TEST(Tests, ContextCapture) {
    Context errorContext;
    ASSERT_THROW(throw_with_detail<SomeDetail>("something"), std::runtime_error);
    ASSERT_TRUE(errorContext.has_detail<SomeDetail>());
    ASSERT_EQ(errorContext.get_detail<SomeDetail>(), "something");

    std::ostringstream s;
    s << errorContext.get_details();
    EXPECT_THAT(s.str(), HasSubstr("  SomeDetail: something"));
}

TEST(Tests, Helper) {
    const auto result = run_with_context(
        [] {
            throw_with_detail<SomeDetail>("something");
            return std::string("wrong");
        },
        [](const Context& context, const std::exception_ptr& eptr) {
            std::ostringstream s;
            try {
                std::rethrow_exception(eptr);
            }
            catch (const std::exception& e) {
                s << "Exception caught:\n  " << e << '\n';
            }
            catch (...) {
            }
            s << context;
            return s.str();
        });
    EXPECT_THAT(
        result,
        HasSubstr("  std::runtime_error: Runtime error description"));
    EXPECT_THAT(result, HasSubstr("  SomeDetail: something"));
    EXPECT_THAT(result, HasSubstr("  throwRuntimeError()"));
}

TEST(Tests, HelperWithVoidFunction) {
    ASSERT_THROW(
        y::error::run_with_context(
            [] { throw_with_detail<SomeDetail>("something"); },
            [](const y::error::Context &, const std::exception_ptr& e) {
            std::rethrow_exception(e);
            }),
        std::runtime_error);
}

TEST(Tests, MainHandler) {
    std::stringstream ss;
    const auto result = run_with_context(
        []() -> int { throwRuntimeError(); },
        make_print_and_return_error_handler(ss, 25));
    ASSERT_EQ(result, 25);
    EXPECT_THAT(ss.str(), HasSubstr("  throwRuntimeError()"));
}

TEST(Tests, ThreadHandler) {
    using namespace std::chrono_literals;
    std::binary_semaphore sem{0};
    std::stringstream ss;
    std::thread t(
        [&] {
            y::error::run_with_context(
                [&]() {
                    sem.release();
                    std::this_thread::sleep_for(10s);
                },
                make_print_and_rethrow_error_handler(ss));
        });
    sem.acquire();
    pthread_cancel(t.native_handle());
    t.join();
    EXPECT_EQ(ss.str(), "");
}
