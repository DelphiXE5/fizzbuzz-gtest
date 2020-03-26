cmake_minimum_required(VERSION 3.10)
project(Fizzbuzz)

set(CMAKE_CXX_STANDARD 14)

enable_testing()

include_directories(src)

add_subdirectory(lib/googletest)
add_subdirectory(src)
add_subdirectory(test)