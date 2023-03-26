#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include "libnetwrk.hpp"
#include "utilities_assert.hpp"

using namespace libnetwrk::net::tcp;

void tcp_client_create() {
	tcp_client<int> client;

	ASSERT(client.connected() == false);
}

void tcp_client_connect_fail() {
	tcp_client<int> client;

	ASSERT(client.connect("127.0.0.1", 21205) == false);
}

void run_all_tests() {
	tcp_client_create();
	tcp_client_connect_fail();
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		tcp_client_create();
		tcp_client_connect_fail();
	}
	else {
		switch (std::stoi(argv[1])) {
			case 0: tcp_client_create();			break;
			case 1: tcp_client_connect_fail();		break;
			default: break;
		}
	}

	return 0;
}