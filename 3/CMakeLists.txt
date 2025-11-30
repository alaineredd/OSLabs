cmake_minimum_required(VERSION 3.10)
project(lab)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=maybe-uninitialized")

if(WIN32)
    message(STATUS "Configuring for Windows")
    add_definitions(-D_WIN32)
else()
    message(STATUS "Configuring for Linux/Unix")
    set(OS_SOURCES os.cpp)
    set(PLATFORM_LIBS pthread rt)
endif()

add_executable(parent
    parent.cpp
    ${OS_SOURCES}
)
target_include_directories(parent PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(parent PRIVATE ${PLATFORM_LIBS})

add_executable(child
    child.cpp
    ${OS_SOURCES}
)
target_include_directories(child PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(child PRIVATE ${PLATFORM_LIBS})