cmake_minimum_required(VERSION 3.13)
project(socket_demo C)

set(CMAKE_C_STANDARD 99)
INCLUDE_DIRECTORIES(tcp-demo)

add_executable(getaddrinfo-demo getaddrinfo-demo/main.c)
add_executable(tcp-select-server tcp-demo/tcp-select-server.c)
add_executable(tcp-client tcp-demo/tcp-client.c)

add_executable(udp-client udp-demo/udp-client.c)
add_executable(udp-server udp-demo/udp-server.c)