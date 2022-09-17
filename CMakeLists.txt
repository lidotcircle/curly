cmake_minimum_required(VERSION 3.10)
project(curly VERSION 0.1.0)

include("cmake/CPM.cmake")
include("cmake/googletest.cmake")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")


add_library(curly INTERFACE)
target_include_directories(curly INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")


add_subdirectory(test)
