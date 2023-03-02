CMAKE_MINIMUM_REQUIRED(VERSION 3.8)

# TEST: MESSAGE
ADD_EXECUTABLE(test_message test_message.cpp)
ADD_TEST(NAME test_message COMMAND test_message)

# TEST: TSDEQUE
ADD_EXECUTABLE(test_tsdeque test_tsdeque.cpp)
ADD_TEST(NAME test_tsdeque COMMAND test_tsdeque)

LINK_LIBRARIES(libnetwrk)
