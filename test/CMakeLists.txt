add_executable(small_vector_test EXCLUDE_FROM_ALL main.cpp)

target_link_libraries(small_vector_test PUBLIC SmallVector::SmallVector)

add_custom_target(check COMMAND ./small_vector_test DEPENDS small_vector_test)
