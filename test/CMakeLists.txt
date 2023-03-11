CMAKE_MINIMUM_REQUIRED(VERSION 3.8)

# TEST: MESSAGE
ADD_EXECUTABLE(test_message test_message.cpp)
ADD_TEST(NAME message_create COMMAND test_message 0)

# TEST: TYPE TRAITS
ADD_EXECUTABLE(test_type_traits test_type_traits.cpp)
ADD_TEST(NAME type_trait_is_iterable COMMAND test_type_traits 0)
ADD_TEST(NAME type_trait_is_iterable_standard COMMAND test_type_traits 1)

# TEST: BINARY_SERIALIZER
ADD_EXECUTABLE(test_binary_serializer test_binary_serializer.cpp)
ADD_TEST(NAME serialize_deserialize_standard_layout COMMAND test_binary_serializer 0)
ADD_TEST(NAME serialize_deserialize_standard_layout_containers COMMAND test_binary_serializer 1)
ADD_TEST(NAME serialize_deserialize_strings COMMAND test_binary_serializer 2)
ADD_TEST(NAME serialize_deserialize_serializable COMMAND test_binary_serializer 3)
ADD_TEST(NAME serialize_deserialize_unsupported COMMAND test_binary_serializer 4)

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

# TEST: BUFFER
ADD_EXECUTABLE(test_buffer test_buffer.cpp)
ADD_TEST(NAME buffer_create COMMAND test_buffer 0)
ADD_TEST(NAME buffer_get_range COMMAND test_buffer 1)
ADD_TEST(NAME buffer_push_back_buffer COMMAND test_buffer 2)
ADD_TEST(NAME buffer_push_at COMMAND test_buffer 3)
ADD_TEST(NAME buffer_get_range_exception COMMAND test_buffer 4)

# TEST: SERIALIZABLE
ADD_EXECUTABLE(test_serializable test_serializable.cpp)
ADD_TEST(NAME serialize_deserialize_simple_struct COMMAND test_serializable 0)

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