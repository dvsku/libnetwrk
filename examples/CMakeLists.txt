INCLUDE_DIRECTORIES("${LIBNETWRK_INCLUDES}")
INCLUDE_DIRECTORIES("${DVUTIL_INCLUDES}")
INCLUDE_DIRECTORIES("${LIBNETWRK_ROOT}/examples")

LINK_LIBRARIES(dv_utilities)

# EXAMPLE: TCP_ECHO
ADD_EXECUTABLE(tcp_echo_server tcp_echo/tcp_echo_service.cpp)
ADD_EXECUTABLE(tcp_echo_client tcp_echo/tcp_echo_client.cpp)
