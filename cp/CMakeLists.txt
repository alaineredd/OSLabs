cmake_minimum_required(VERSION 3.10)
project(cp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Сервер
add_executable(server server.cpp os.cpp)
target_include_directories(server PRIVATE .)
target_compile_options(server PRIVATE -pthread)
target_link_libraries(server PRIVATE pthread rt)

# Клиент
add_executable(client client.cpp os.cpp)
target_include_directories(client PRIVATE .)
target_compile_options(client PRIVATE -pthread)
target_link_libraries(client PRIVATE pthread rt)

# Устанавливаем выходную директорию
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})