CMAKE_MINIMUM_REQUIRED(VERSION 3.8)

ADD_LIBRARY(libnetwrk STATIC 
	"include/libnetwrk.hpp"
	"include/net/definitions.hpp"
	"include/net/common/message.hpp"
	"include/net/common/connection.hpp"
	"include/net/common/serialization/serializable.hpp"
	"include/net/common/serialization/serialize.hpp"
	"include/net/common/serialization/type_traits.hpp"
	"include/net/common/containers/tsdeque.hpp"
	"include/utilities/buffers.hpp"
	"include/net/tcp/tcp_server.hpp"
	"include/net/tcp/tcp_client.hpp"

	"impl/libnetwrk.cpp" 
	"impl/utilities/buffers.cpp")

SET_TARGET_PROPERTIES(libnetwrk PROPERTIES LINKER_LANGUAGE CXX)