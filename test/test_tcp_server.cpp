#include "net/tcp/tcp_server.hpp"

#include <cassert>

using namespace libnetwrk::net::tcp;

void tcp_server_create() {
	tcp_server<int> server;

	assert(server.running() == false);
}

void tcp_server_start() {
	tcp_server<int> server;
	server.start("127.0.0.1", 21205);

	assert(server.running() == true);
}

void tcp_server_stop() {
	tcp_server<int> server;
	server.start("127.0.0.1", 21205);

	assert(server.running() == true);

	server.stop();

	assert(server.running() == false);
}

void tcp_server_async_process_messages() {
	tcp_server<int> server;
	server.start("127.0.0.1", 21205);
	server.async_process_messages();
}

void run_all_tests() {
	tcp_server_create();
	tcp_server_start();
	tcp_server_stop();
	tcp_server_async_process_messages();
}

int main(int argc, char* argv[]) {
	run_all_tests();
	return 0;
}