cmake_minimum_required(VERSION 3.5)
project(SmallVector VERSION 0.1.0)

# config options
option(SMALL_VECTOR_ENABLE_TESTS "Enable unit tests" ON)

# compiler warnings
if (MSVC)
    add_compile_options("/W4")
else()
    add_compile_options("-Wpedantic")
    add_compile_options("-Wall")
    add_compile_options("-Wextra")
endif()

# add functional library
include(CheckCXXCompilerFlag)

add_library(SmallVector INTERFACE)
add_library(SmallVector::SmallVector ALIAS SmallVector)

target_compile_features(SmallVector INTERFACE cxx_std_17)

target_include_directories(SmallVector INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

# add tests
if (SMALL_VECTOR_ENABLE_TESTS)
    add_subdirectory(test)
endif()
