CMAKE_MINIMUM_REQUIRED (VERSION 3.8)

PROJECT (libnetwrk_proj C CXX)

IF(CMAKE_VERSION VERSION_GREATER 3.12) 
    IF(MSVC)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20 -D_WIN32_WINNT=0x0601")
    ELSE()
        SET(CMAKE_CXX_STANDARD 20)
    ENDIF()
ELSE()
    IF(MSVC)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 -D_WIN32_WINNT=0x0601")
    ELSE()
        SET(CMAKE_CXX_STANDARD 17)
    ENDIF()
ENDIF()

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/libnetwrk")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/libnetwrk/lib/asio")

ADD_LIBRARY(libnetwrk STATIC 
	"libnetwrk/libnetwrk.hpp"
	"libnetwrk/libnetwrk/net/definitions.hpp"
	"libnetwrk/libnetwrk/net/message.hpp"
	"libnetwrk/libnetwrk/net/common/base_connection.hpp"
	"libnetwrk/libnetwrk/net/common/serialization/serializable.hpp"
	"libnetwrk/libnetwrk/net/common/serialization/type_traits.hpp"
	"libnetwrk/libnetwrk/net/common/containers/tsdeque.hpp"
	"libnetwrk/libnetwrk/net/tcp/tcp_connection.hpp"
	"libnetwrk/libnetwrk/net/tcp/tcp_server.hpp"
	"libnetwrk/libnetwrk/net/tcp/tcp_client.hpp")

SET_TARGET_PROPERTIES(libnetwrk PROPERTIES LINKER_LANGUAGE CXX)

ENABLE_TESTING()

ADD_SUBDIRECTORY ("test")