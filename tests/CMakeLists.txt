add_executable(error-context-tests main.cpp tests.cpp throwRuntimeError.cpp)
find_package(GTest REQUIRED)

target_link_libraries(error-context-tests
        PRIVATE
        GTest::GTest
        error-context)

add_test(error-context error-context-tests)