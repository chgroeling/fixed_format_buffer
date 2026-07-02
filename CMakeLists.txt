cmake_minimum_required(VERSION 3.15)
project(fixed_format_buffer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(GTest REQUIRED)

add_library(fixed_format_buffer INTERFACE)
target_include_directories(fixed_format_buffer INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(fixed_format_buffer INTERFACE cxx_std_17)

enable_testing()
add_subdirectory(tests)

find_package(Doxygen QUIET)
if(Doxygen_FOUND)
    doxygen_add_docs(doc
        include/
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        COMMENT "Generate API documentation"
    )
endif()
