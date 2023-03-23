CMAKE_MINIMUM_REQUIRED(VERSION 3.8)

# EXAMPLE: TCP_ECHO
ADD_EXECUTABLE(tcp_echo_server tcp_echo/tcp_echo_server.cpp)
ADD_EXECUTABLE(tcp_echo_client tcp_echo/tcp_echo_client.cpp)
