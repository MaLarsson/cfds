# Clone Catch2 repository at configure time
find_package(Git)

if(NOT GIT_FOUND)
    message(FATAL_ERROR "git not found!")
endif()

set(CATCH2_URL "https://github.com/catchorg/Catch2.git")
set(LIB_DIR "${CMAKE_BINARY_DIR}/lib")
set(CATCH2_DIR "${LIB_DIR}/Catch2")

file(MAKE_DIRECTORY ${LIB_DIR})

if(EXISTS ${CATCH2_DIR})
    execute_process(
        COMMAND ${GIT_EXECUTABLE} pull origin master
        WORKING_DIRECTORY ${LIB_DIR}
        OUTPUT_VARIABLE git_output)
else()
    execute_process(
        COMMAND ${GIT_EXECUTABLE} clone ${CATCH2_URL}
        WORKING_DIRECTORY ${LIB_DIR}
        OUTPUT_VARIABLE git_output)
endif()

message("${git_output}")

# Add Catch2, this defines the Catch2::Catch2 target.
add_subdirectory(${CATCH2_DIR} ${CATCH2_DIR} EXCLUDE_FROM_ALL)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CATCH2_DIR}/contrib")

# Add check target
add_executable(run_test EXCLUDE_FROM_ALL main.cpp test.cpp traits.cpp)

if (NOT MSVC)
    if (SMALL_VECTOR_ENABLE_ASAN)
        target_compile_options(run_test PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
        target_link_libraries(run_test PRIVATE -fsanitize=address -g)
    endif()

    if (SMALL_VECTOR_ENABLE_UBSAN)
        target_compile_options(run_test PRIVATE -fsanitize=undefined)
        target_link_libraries(run_test PRIVATE -fsanitize=undefined)
    endif()

    if (SMALL_VECTOR_ENABLE_MSAN)
        target_compile_options(run_test PRIVATE -fsanitize=memory -fno-omit-frame-pointer -g)
        target_link_libraries(run_test PRIVATE -fsanitize=memory -g)
    endif()
endif()

target_link_libraries(run_test PUBLIC SmallVector::SmallVector Catch2::Catch2)

include(Catch)
catch_discover_tests(run_test)

add_custom_target(check COMMAND run_test DEPENDS run_test)
