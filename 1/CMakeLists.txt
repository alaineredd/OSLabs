cmake_minimum_required(VERSION 3.10)
project(OS_lab1)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(WIN32)
    message(STATUS "Configuring for Windows")
    add_definitions(-D_WIN32)
else()
    message(STATUS "Configuring for Linux/Unix")
    set(OS_SOURCES os_linux.c)
    set(PLATFORM_LIBS)
endif()

add_executable(parent parent.c ${OS_SOURCES})
target_include_directories(parent PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(parent PRIVATE ${PLATFORM_LIBS})

add_executable(child child.c ${OS_SOURCES})
target_include_directories(child PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(child PRIVATE ${PLATFORM_LIBS})