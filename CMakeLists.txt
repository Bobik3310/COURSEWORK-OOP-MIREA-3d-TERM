cmake_minimum_required(VERSION 3.12)
project(TwoThreeTree LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(twothree src/TwoThreeTree.cpp)
target_include_directories(twothree PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(twothree PRIVATE -Wall -Wextra -Wpedantic)

add_executable(demo src/main.cpp)
target_link_libraries(demo PRIVATE twothree)

include(CTest)
if(BUILD_TESTING)
    add_executable(tree_tests tests/test.cpp)
    target_link_libraries(tree_tests PRIVATE twothree)
    add_test(NAME tree_randomized COMMAND tree_tests)
endif()
