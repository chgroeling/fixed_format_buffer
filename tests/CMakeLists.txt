cmake_minimum_required(VERSION 3.15)

find_package(GTest REQUIRED)

add_executable(fixed_format_buffer_tests
    fixed_format_buffer_test.cpp
    buffer_policy_test.cpp
    sprintf_comparison_test.cpp
)
target_link_libraries(fixed_format_buffer_tests PRIVATE
    fixed_format_buffer
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(fixed_format_buffer_tests)
