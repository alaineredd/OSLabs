cmake_minimum_required(VERSION 3.10)
project(lab4)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_library(gcd_sin1 SHARED 
    include/lib1.cpp
    include/mathlib.h
)

add_library(gcd_sin2 SHARED 
    include/lib2.cpp
    include/mathlib.h
)

set_target_properties(gcd_sin1 PROPERTIES OUTPUT_NAME "gcd_sin1")
set_target_properties(gcd_sin2 PROPERTIES OUTPUT_NAME "gcd_sin2")

add_executable(program1 include/program1.cpp)
target_include_directories(program1 PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(program1 gcd_sin1 m)

add_executable(program2 include/program2.cpp)
target_include_directories(program2 PRIVATE ${CMAKE_SOURCE_DIR})

if(UNIX)
    target_link_libraries(program2 dl m)
endif()

add_custom_command(TARGET program2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:gcd_sin1>
    ${CMAKE_BINARY_DIR}/bin/$<TARGET_FILE_NAME:gcd_sin1>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:gcd_sin2>
    ${CMAKE_BINARY_DIR}/bin/$<TARGET_FILE_NAME:gcd_sin2>
)
