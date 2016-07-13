cmake_minimum_required(VERSION 3.5)
project(chickadee)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(include src)
set(SOURCE_FILES src/main.cpp include/lexer.h src/lexer.cpp include/ast.h include/parser.h src/parser.cpp include/helper.h src/toplevel.cpp include/toplevel.h src/codegen.cpp include/codegen.h)
add_executable(chickadee ${SOURCE_FILES})

find_package(LLVM REQUIRED CONFIG)
if(LLVM_FOUND)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
    include_directories(${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})

    # Find the libraries that correspond to the LLVM components  that we wish to use
    llvm_map_components_to_libnames(llvm_libs support core irreader)

    # Link against LLVM libraries
    target_link_libraries(chickadee ${llvm_libs})
endif()