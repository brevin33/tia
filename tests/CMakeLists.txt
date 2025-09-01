cmake_minimum_required(VERSION 3.15)
project(tests C)

set(CMAKE_C_STANDARD 11)

add_executable(tia_tests src/tests.c)

target_link_libraries(tia_tests 
	tia	
)
