cmake_minimum_required(VERSION 3.0)
project(AAS_practice VERSION 1.0.0.0 DESCRIPTION "Practice for algorithms and data structures." HOMEPAGE_URL "https://github.com/ntllct/AAS_practice" LANGUAGES "CXX")


option(CMAKE_VERBOSE_MAKEFILE "VERBOSE MODE" OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

enable_testing()

add_subdirectory(AVLTree)