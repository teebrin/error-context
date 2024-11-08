add_executable(error-context-tests tests.cpp throwRuntimeError.cpp)
find_package(GTest REQUIRED)

target_link_libraries(error-context-tests
        PRIVATE
        GTest::GTest
        GTest::gtest_main
        error-context)

add_test(error-context error-context-tests)