cmake_minimum_required(VERSION 3.5)
project(at_tty_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(at_server
  src/main.cpp
  src/pattern_matcher.cpp
  src/dictionary.cpp
  src/tty_server.cpp
)

target_include_directories(at_server PRIVATE src)

enable_testing()
add_executable(matcher_tests tests/matcher_tests.cpp src/pattern_matcher.cpp)
target_include_directories(matcher_tests PRIVATE src)
add_test(NAME matcher_tests COMMAND matcher_tests)
