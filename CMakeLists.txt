cmake_minimum_required(VERSION 3.16)
project(error-context LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_ENABLE_EXPORTS ON)

enable_testing()

add_subdirectory(sources)
add_subdirectory(tests)