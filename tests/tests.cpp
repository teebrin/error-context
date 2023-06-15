#include <y/error.h>
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "details.h"
#include "throwRuntimeError.h"

using namespace testing;

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
                [](const y::error::Context& context, std::exception_ptr e) { throw; }),
            std::runtime_error);
}