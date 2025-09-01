cmake_minimum_required(VERSION 3.15)
project(Tia C)

set(CMAKE_C_STANDARD 11)

# find_package(LLVM CONFIG REQUIRED)
#
# list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
# include(HandleLLVMOptions)
# add_definitions(${LLVM_DEFINITIONS})

# llvm_map_components_to_libnames(llvm_libs Support Core IRReader)

add_library(tia
	src/tia.c
	src/basic.c
	src/arena.c
	src/file.c
	src/lists.c
	src/folder.c
	src/token.c
	src/logging.c
	src/ast.c
	src/type.c
	src/function.c
	src/statement.c
	src/expression.c
	src/scope.c
)

target_include_directories(tia
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        # ${LLVM_INCLUDE_DIRS}
)

target_compile_definitions(tia
	PUBLIC
        # ${LLVM_DEFINITIONS}
)

# Link with LLVM C API libraries
target_link_libraries(tia
	PUBLIC
	 #    ${llvm_libs}
		# LLVM-C
)
