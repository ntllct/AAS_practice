add_executable(AVLTreeTest main.cpp)
install(TARGETS AVLTreeTest DESTINATION bin)

add_test(NAME AVL_test_1 COMMAND AVLTreeTest)
set_tests_properties(AVL_test_1 PROPERTIES PASS_REGULAR_EXPRESSION "-7 -1 2 5 5 19 503 1112 ")