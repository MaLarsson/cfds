add_executable(run_test EXCLUDE_FROM_ALL main.cpp)

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

target_link_libraries(run_test PUBLIC SmallVector::SmallVector)

add_custom_target(check COMMAND run_test DEPENDS run_test)
