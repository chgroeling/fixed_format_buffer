cmake_minimum_required(VERSION 3.15)
project(test_package LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(fixed_format_buffer REQUIRED)

add_executable(test_package src/test.cpp)
target_link_libraries(test_package fixed_format_buffer::fixed_format_buffer)
