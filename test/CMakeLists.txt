add_executable(run_test EXCLUDE_FROM_ALL main.cpp)

if (!MSVC)
    target_compile_options(run_test PRIVATE -fno-omit-frame-pointer -fsanitize=address -g)
    target_link_libraries(run_test PRIVATE -fsanitize=address -g)
endif()

target_link_libraries(run_test PUBLIC SmallVector::SmallVector)

add_custom_target(check COMMAND run_test DEPENDS run_test)
