# Clone Google Benchmark repository at configure time
find_package(Git)

if(NOT GIT_FOUND)
  message(FATAL_ERROR "git not found!")
endif()

set(GBENCH_URL "https://github.com/google/benchmark.git")
set(LIB_DIR "${CMAKE_BINARY_DIR}/lib")
set(GBENCH_DIR "${LIB_DIR}/benchmark")

file(MAKE_DIRECTORY ${LIB_DIR})
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Suppressing benchmark's tests" FORCE)

if(EXISTS ${GBENCH_DIR})
  execute_process(
    COMMAND ${GIT_EXECUTABLE} pull origin master
    WORKING_DIRECTORY ${LIB_DIR}
    OUTPUT_VARIABLE git_output)
else()
  execute_process(
    COMMAND ${GIT_EXECUTABLE} clone ${GBENCH_URL}
    WORKING_DIRECTORY ${LIB_DIR}
    OUTPUT_VARIABLE git_output)
endif()

message("${git_output}")

# Add Google Benchmark, this defines the benchmark target.
add_subdirectory(${GBENCH_DIR} ${GBENCH_DIR} EXCLUDE_FROM_ALL)

add_executable(small_vector_bench EXCLUDE_FROM_ALL main.cpp)

target_link_libraries(small_vector_bench PUBLIC
    SmallVector::SmallVector benchmark)

add_custom_target(bench COMMAND ./small_vector_bench DEPENDS small_vector_bench)
