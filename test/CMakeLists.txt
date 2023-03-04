CMAKE_MINIMUM_REQUIRED(VERSION 3.8)

# TEST: MESSAGE
ADD_EXECUTABLE(test_message test_message.cpp)
ADD_TEST(NAME message_create COMMAND test_message 0)

# TEST: TSDEQUE
ADD_EXECUTABLE(test_tsdeque test_tsdeque.cpp)
ADD_TEST(NAME tsdeque_create COMMAND test_tsdeque 0)
ADD_TEST(NAME tsdeque_push_front COMMAND test_tsdeque 1)
ADD_TEST(NAME tsdeque_push_back COMMAND test_tsdeque 2)
ADD_TEST(NAME tsdeque_front COMMAND test_tsdeque 3)
ADD_TEST(NAME tsdeque_back COMMAND test_tsdeque 4)
ADD_TEST(NAME tsdeque_pop_front COMMAND test_tsdeque 5)
ADD_TEST(NAME tsdeque_pop_back COMMAND test_tsdeque 6)
ADD_TEST(NAME tsdeque_clear COMMAND test_tsdeque 7)
ADD_TEST(NAME tsdeque_empty COMMAND test_tsdeque 8)
ADD_TEST(NAME tsdeque_count COMMAND test_tsdeque 9)

# TEST: SERIALIZABLE
ADD_EXECUTABLE(test_serializable test_serializable.cpp)
ADD_TEST(NAME serialize_deserialize_custom_struct COMMAND test_serializable 0)

# TEST: TCP_SERVER
ADD_EXECUTABLE(test_tcp_server test_tcp_server.cpp)
ADD_TEST(NAME tcp_server_create COMMAND test_tcp_server 0)
ADD_TEST(NAME tcp_server_start_async COMMAND test_tcp_server 1)
ADD_TEST(NAME tcp_server_start_async_twice COMMAND test_tcp_server 2)
ADD_TEST(NAME tcp_server_stop COMMAND test_tcp_server 3)

# TEST: TCP_CLIENT
ADD_EXECUTABLE(test_tcp_client test_tcp_client.cpp)
ADD_TEST(NAME tcp_client_create COMMAND test_tcp_client 0)
ADD_TEST(NAME tcp_client_connect_fail COMMAND test_tcp_client 1)

# TEST: TCP_SERVER_CLIENT
ADD_EXECUTABLE(test_tcp_server_client test_tcp_server_client.cpp)
ADD_TEST(NAME service_connect COMMAND test_tcp_server_client 0)
ADD_TEST(NAME service_client_hello COMMAND test_tcp_server_client 1)
ADD_TEST(NAME service_echo COMMAND test_tcp_server_client 2)
ADD_TEST(NAME service_ping_pong COMMAND test_tcp_server_client 3)
ADD_TEST(NAME service_broadcast COMMAND test_tcp_server_client 4)

LINK_LIBRARIES(libnetwrk)
