#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include "libnetwrk.hpp"
#include "utilities_assert.hpp"

using namespace libnetwrk::net::tcp;

enum class commands : int {
	hello
};

void tcp_server_create() {
	ASSERT_NOT_THROWS_CTOR(tcp_server<commands> server);
	ASSERT(!server.running());
}

void tcp_server_start_ip() {
	ASSERT_NOT_THROWS_CTOR(tcp_server<commands> server);
	ASSERT(server.start("127.0.0.1", 21205));
	ASSERT(server.running());
}

void tcp_server_start_endpoint_localhost() {
	ASSERT_NOT_THROWS_CTOR(tcp_server<commands> server);
	ASSERT(server.start(libnetwrk::endpoint::localhost, 21205));
	ASSERT(server.running());
}

void tcp_server_start_endpoint_v4() {
	ASSERT_NOT_THROWS_CTOR(tcp_server<commands> server);
	ASSERT(server.start(libnetwrk::endpoint::v4, 21205));
	ASSERT(server.running());
}

void tcp_server_start_twice() {
	ASSERT_NOT_THROWS_CTOR(tcp_server<commands> server);
	ASSERT(server.start("127.0.0.1", 21205));
	ASSERT(!server.start("127.0.0.1", 21205));
	ASSERT(server.running());
}

void tcp_server_stop() {
	ASSERT_NOT_THROWS_CTOR(tcp_server<commands> server);
	server.start("127.0.0.1", 21205);
	ASSERT(server.running());
	server.stop();
	ASSERT(!server.running());
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		tcp_server_create();
		tcp_server_start_ip();
		tcp_server_start_endpoint_localhost();
		tcp_server_start_endpoint_v4();
		tcp_server_start_twice();
		tcp_server_stop();
	}
	else {
		switch (std::stoi(argv[1])) {
			case 0: tcp_server_create();							break;
			case 1: tcp_server_start_ip();							break;
			case 2: tcp_server_start_endpoint_localhost();			break;
			case 3: tcp_server_start_endpoint_v4();					break;
			case 4: tcp_server_start_twice();						break;
			case 5: tcp_server_stop();								break;
			default: break;
		}
	}

	return 0;
}