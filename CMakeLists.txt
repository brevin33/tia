cmake_minimum_required(VERSION 3.15)
project(Tia)

add_compile_options(-Werror)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -MT")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -MT")

add_subdirectory(tia)
add_subdirectory(tests)
add_subdirectory(scratch)
