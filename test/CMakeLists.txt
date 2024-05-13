INCLUDE_DIRECTORIES("${LIBNETWRK_INCLUDES}")
INCLUDE_DIRECTORIES("${GTEST_INCLUDES}")

INCLUDE(GoogleTest)
LINK_LIBRARIES(GTest::gtest_main)

ADD_EXECUTABLE(test_bin_serializer test_binary_serializer.cpp)
gtest_discover_tests(test_bin_serializer)

ADD_EXECUTABLE(test_buffer test_buffer.cpp)
gtest_discover_tests(test_buffer)

ADD_EXECUTABLE(test_serializable test_serializable.cpp)
gtest_discover_tests(test_serializable)

ADD_EXECUTABLE(test_service_client test_service_client.cpp)
gtest_discover_tests(test_service_client)

ADD_EXECUTABLE(test_tcp_client test_tcp_client.cpp)
gtest_discover_tests(test_tcp_client)

ADD_EXECUTABLE(test_tcp_command_type test_tcp_command_type.cpp)
gtest_discover_tests(test_tcp_command_type)

ADD_EXECUTABLE(test_tcp_service test_tcp_service.cpp)
gtest_discover_tests(test_tcp_service)

ADD_EXECUTABLE(test_tcp_service_client test_tcp_service_client.cpp)
gtest_discover_tests(test_tcp_service_client)

ADD_EXECUTABLE(test_tcp_talk test_tcp_talk.cpp)
gtest_discover_tests(test_tcp_talk)
