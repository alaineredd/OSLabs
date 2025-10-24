cmake_minimum_required(VERSION 3.10)
project(lab3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=maybe-uninitialized")

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

add_library(thread_lib src/thread.cpp)
add_library(matrix_lib src/matrix.cpp)

target_link_libraries(matrix_lib PUBLIC thread_lib)

target_include_directories(thread_lib PUBLIC include)
target_include_directories(matrix_lib PUBLIC include)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE thread_lib matrix_lib)