cmake_minimum_required(VERSION 3.5)
project(SmallVector VERSION 0.1.0)

# config options
option(SMALL_VECTOR_ENABLE_TESTS "Enable unit tests" ON)
option(SMALL_VECTOR_ENABLE_BENCHMARKS "Enable benchmarks" OFF)
option(SMALL_VECTOR_ENABLE_ASAN "Enable address sanitizer" OFF)
option(SMALL_VECTOR_ENABLE_UBSAN "Enable undefined behaviour sanitizer" OFF)
option(SMALL_VECTOR_ENABLE_MSAN "Enable memory sanitizer" OFF)

# compiler warnings
if (MSVC)
    add_compile_options("/W3")
else()
    add_compile_options("-Wpedantic")
    add_compile_options("-Wall")
    add_compile_options("-Wextra")
endif()

# add functional library
include(CheckCXXCompilerFlag)

add_library(SmallVector INTERFACE)
add_library(SmallVector::SmallVector ALIAS SmallVector)

target_compile_features(SmallVector INTERFACE cxx_std_11)

target_include_directories(SmallVector INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

# add tests
if (SMALL_VECTOR_ENABLE_TESTS)
    add_subdirectory(test)
endif()

# add benchmarks
if (SMALL_VECTOR_ENABLE_BENCHMARKS)
    add_subdirectory(benchmark)
endif()
