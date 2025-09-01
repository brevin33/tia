cmake_minimum_required(VERSION 3.15)
project(scratch C)

set(CMAKE_C_STANDARD 11)

add_executable(tia_scratch src/scratch.c)

target_link_libraries(tia_scratch 
	tia	
)
