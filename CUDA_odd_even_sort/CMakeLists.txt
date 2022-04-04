add_executable(cuda_odd_even_sort main.cu)
install(TARGETS cuda_odd_even_sort DESTINATION bin)
target_include_directories(cuda_odd_even_sort PUBLIC "${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}/../../../samples/common/inc/")

add_test(NAME cuda_odd_even_sort_1 COMMAND cuda_odd_even_sort)
set_tests_properties(cuda_odd_even_sort_1 PROPERTIES PASS_REGULAR_EXPRESSION "ok")