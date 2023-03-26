#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include "libnetwrk.hpp"

#include <cassert>

using namespace libnetwrk::net::tcp;

void tcp_server_create() {
	tcp_server<int> server;

	assert(server.running() == false);
}

void tcp_server_start_async() {
	tcp_server<int> server;
	
	assert(server.start("127.0.0.1", 21205) == true);
	assert(server.running() == true);
}

void tcp_server_start_async_twice() {
	tcp_server<int> server;

	assert(server.start("127.0.0.1", 21205) == true);
	assert(server.start("127.0.0.1", 21205) == false);
	assert(server.running() == true);
}

void tcp_server_stop() {
	tcp_server<int> server;
	server.start("127.0.0.1", 21205);

	assert(server.running() == true);

	server.stop();

	assert(server.running() == false);
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		tcp_server_create();
		tcp_server_start_async();
		tcp_server_start_async_twice();
		tcp_server_stop();
	}
	else {
		switch (std::stoi(argv[1])) {
			case 0: tcp_server_create();				break;
			case 1: tcp_server_start_async();			break;
			case 2: tcp_server_start_async_twice();		break;
			case 3: tcp_server_stop();					break;
			default: break;
		}
	}

	return 0;
}