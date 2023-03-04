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

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src")

ADD_LIBRARY(libnetwrk STATIC 
	"src/libnetwrk.hpp"
	"src/libnetwrk/net/definitions.hpp"
	"src/libnetwrk/net/message.hpp"
	"src/libnetwrk/net/common/base_connection.hpp"
	"src/libnetwrk/net/common/serialization/serializable.hpp"
	"src/libnetwrk/net/common/serialization/serialize.hpp"
	"src/libnetwrk/net/common/serialization/type_traits.hpp"
	"src/libnetwrk/net/common/containers/tsdeque.hpp"
	"src/libnetwrk/utilities/buffers.hpp"
	"src/libnetwrk/net/tcp/tcp_connection.hpp"
	"src/libnetwrk/net/tcp/tcp_server.hpp"
	"src/libnetwrk/net/tcp/tcp_client.hpp")

SET_TARGET_PROPERTIES(libnetwrk PROPERTIES LINKER_LANGUAGE CXX)

ENABLE_TESTING()

ADD_SUBDIRECTORY ("test")