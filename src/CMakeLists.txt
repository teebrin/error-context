
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "include/y/*.h")

add_library(error-context SHARED)
target_sources(error-context
        PRIVATE error.cpp demangle.cpp stacktrace.cpp
        PUBLIC ${HEADER_LIST})
target_include_directories(error-context PUBLIC include)
target_compile_features(error-context PUBLIC cxx_std_17)