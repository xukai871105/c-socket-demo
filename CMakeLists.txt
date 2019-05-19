cmake_minimum_required(VERSION 3.13)
project(socket_demo C)

set(CMAKE_C_STANDARD 99)
INCLUDE_DIRECTORIES(tcp-demo)

add_executable(getaddrinfo-demo getaddrinfo-demo/main.c)
add_executable(tcp-server tcp-demo/server.c)
add_executable(tcp-client tcp-demo/client.c)