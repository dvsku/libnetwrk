CMAKE_MINIMUM_REQUIRED(VERSION 3.8)

# TEST: MESSAGE
ADD_EXECUTABLE(test_message test_message.cpp)
ADD_TEST(NAME test_message COMMAND test_message)

# TEST: TSDEQUE
ADD_EXECUTABLE(test_tsdeque test_tsdeque.cpp)
ADD_TEST(NAME test_tsdeque COMMAND test_tsdeque)

# TEST: TCP_SERVER
ADD_EXECUTABLE(test_tcp_server test_tcp_server.cpp)
ADD_TEST(NAME test_tcp_server COMMAND test_tcp_server)

# TEST: TCP_CLIENT
ADD_EXECUTABLE(test_tcp_client test_tcp_client.cpp)
ADD_TEST(NAME test_tcp_client COMMAND test_tcp_client)

LINK_LIBRARIES(libnetwrk)
