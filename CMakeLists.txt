cmake_minimum_required(VERSION 3.15)

set(FFB_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/ffb/fixed_format_buffer.h")
file(READ "${FFB_HEADER}" _ffb_hdr)
string(REGEX MATCH "#define FFB_VERSION_MAJOR ([0-9]+)" _ "${_ffb_hdr}")
set(_ffb_major ${CMAKE_MATCH_1})
string(REGEX MATCH "#define FFB_VERSION_MINOR ([0-9]+)" _ "${_ffb_hdr}")
set(_ffb_minor ${CMAKE_MATCH_1})
string(REGEX MATCH "#define FFB_VERSION_PATCH ([0-9]+)" _ "${_ffb_hdr}")
set(_ffb_patch ${CMAKE_MATCH_1})
set(FFB_VERSION "${_ffb_major}.${_ffb_minor}.${_ffb_patch}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${FFB_HEADER}")

project(fixed_format_buffer VERSION ${FFB_VERSION} LANGUAGES CXX)

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
    set(DOXYGEN_PROJECT_NUMBER ${FFB_VERSION})
    doxygen_add_docs(doc
        include/
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        COMMENT "Generate API documentation"
    )
endif()
