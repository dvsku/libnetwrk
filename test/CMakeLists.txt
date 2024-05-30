INCLUDE_DIRECTORIES("${GTEST_INCLUDES}")

INCLUDE(GoogleTest)
LINK_LIBRARIES(libnetwrk)
LINK_LIBRARIES(GTest::gtest_main)

ADD_EXECUTABLE(test_serialize_fixed   test_serialize.cpp)
ADD_EXECUTABLE(test_serialize_dynamic test_serialize.cpp)

TARGET_COMPILE_DEFINITIONS(test_serialize_dynamic PRIVATE -DLIBNETWRK_SERIALIZE_TEST_BUFFER_DYNAMIC)
TARGET_COMPILE_DEFINITIONS(test_serialize_dynamic PRIVATE -DLIBNETWRK_ARCHITECTURE_DIR="${PROJECT_SOURCE_DIR}/test/architecture")
TARGET_COMPILE_DEFINITIONS(test_serialize_fixed   PRIVATE -DLIBNETWRK_ARCHITECTURE_DIR="${PROJECT_SOURCE_DIR}/test/architecture")

gtest_discover_tests(test_serialize_fixed)
gtest_discover_tests(test_serialize_dynamic)

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
