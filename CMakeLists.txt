cmake_minimum_required(VERSION 3.12)
project(sysnet LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")

if(MSVC)
    add_compile_options(/W4 /permissive- /WX /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

add_library(tcp_socket STATIC
    src/tcp_socket.cpp
    src/tcp_server.cpp
)
target_include_directories(tcp_socket PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(WIN32)
    target_link_libraries(tcp_socket PUBLIC ws2_32)
endif()

add_subdirectory(client_server)
