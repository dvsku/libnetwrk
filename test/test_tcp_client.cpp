#include "net/tcp/tcp_client.hpp"

#include <cassert>

using namespace libnetwrk::net::tcp;

void tcp_client_create() {
	tcp_client<int> client;

	assert(client.running() == false);
}

void tcp_client_connect_fail() {
	tcp_client<int> client;

	assert(client.connect("127.0.0.1", 21205) == false);
}

void run_all_tests() {
	tcp_client_create();
	tcp_client_connect_fail();
}

int main(int argc, char* argv[]) {
	run_all_tests();
	return 0;
}