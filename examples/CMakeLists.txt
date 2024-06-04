INCLUDE_DIRECTORIES("${LIBNETWRK_ROOT}/examples")

LINK_LIBRARIES(libutil)
LINK_LIBRARIES(libnetwrk)

# EXAMPLE: TCP_ECHO
ADD_EXECUTABLE(tcp_echo_service tcp_echo/tcp_echo_service.cpp)
ADD_EXECUTABLE(tcp_echo_client tcp_echo/tcp_echo_client.cpp)
