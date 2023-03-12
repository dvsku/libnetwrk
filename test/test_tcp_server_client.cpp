#include "libnetwrk.hpp"

#include <thread>
#include <chrono>

#include <cassert>

using namespace libnetwrk::net::tcp;
using namespace libnetwrk::net;

enum class commands : unsigned int {
	c2s_hello,
	c2s_echo,
	s2c_echo,
	c2s_ping,
	s2c_pong,
	c2s_broadcast,
	s2c_broadcast
};

class test_service : public tcp_server<commands> {
	public:
		test_service() : tcp_server() {}

		bool client_said_hello = false;
		bool client_said_echo = false;
		bool client_said_broadcast = false;
		std::string ping = "";
		
		void on_message(owned_message_t& msg) override {
			message<commands> response;
			switch (msg.m_msg.m_head.m_command) {
				case commands::c2s_hello:
					client_said_hello = true;
					break;
				case commands::c2s_echo:
					client_said_echo = true;
					response.m_head.m_command = commands::s2c_echo;
					msg.m_client->send(response);
					break;
				case commands::c2s_ping:
					msg.m_msg >> ping;
					response.m_head.m_command = commands::s2c_pong;
					response << std::string("pOnG");
					msg.m_client->send(response);
					break;
				case commands::c2s_broadcast:
					client_said_broadcast = true;
					response.m_head.m_command = commands::s2c_broadcast;
					send_all(response);
					break;
				default:
					break;
			}
		}

		bool on_client_connect(tcp_connection_t_ptr client) override {
			return true;
		}

		void on_client_disconnect(tcp_connection_t_ptr client) override {

		}

		void wait_for_msg(const int timeout = 30) {
			int tries = 0;
			while (tries < timeout) {
				if (process_single_message()) break;

				tries++;
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}
		}
};

class test_client : public tcp_client<commands> {
	public:
		test_client() : tcp_client() {}

		bool server_said_echo = false;
		bool server_said_broadcast = false;
		std::string pong = "";

		void on_message(message<commands>& msg) override {
			switch (msg.m_head.m_command) {
				case commands::s2c_echo:
					server_said_echo = true;
					break;
				case commands::s2c_pong:
					msg >> pong;
					break;
				case commands::s2c_broadcast:
					server_said_broadcast = true;
					break;

				default:
					break;
			}
		}

		void on_disconnect() override {

		}

		void wait_for_msg(const int timeout = 30) {
			int tries = 0;
			while (tries < timeout) {
				if (process_single_message()) break;

				tries++;
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}
		}
};

void service_connect() {
	test_service server;
	server.start_async("127.0.0.1", 21205);

	test_client client;
	bool connected = client.connect_async("127.0.0.1", 21205);

	assert(connected == true);
}

void service_client_hello() {
	test_service server;
	server.start_async("127.0.0.1", 21205, false);

	test_client client;
	client.connect_async("127.0.0.1", 21205);

	message<commands> msg(commands::c2s_hello);
	client.send(msg);

	server.wait_for_msg();
	assert(server.client_said_hello == true);
}

void service_echo() {
	test_service server;
	server.start_async("127.0.0.1", 21205, false);

	test_client client;
	client.connect_async("127.0.0.1", 21205, false);

	message<commands> msg(commands::c2s_echo);
	client.send(msg);

	server.wait_for_msg();
	assert(server.client_said_echo == true);

	client.wait_for_msg();
	assert(client.server_said_echo == true);
}

void service_ping_pong() {
	test_service server;
	server.start_async("127.0.0.1", 21205, false);

	test_client client;
	client.connect_async("127.0.0.1", 21205, false);

	message<commands> msg(commands::c2s_ping);
	msg << std::string("PiNg");
	client.send(msg);

	server.wait_for_msg();
	assert(server.ping == "PiNg");

	client.wait_for_msg();
	assert(client.pong == "pOnG");
}

void service_broadcast() {
	test_service server;
	server.start_async("127.0.0.1", 21205, false);

	test_client client1;
	assert(client1.connect_async("127.0.0.1", 21205, false) == true);

	test_client client2;
	assert(client2.connect_async("127.0.0.1", 21205, false) == true);

	message<commands> msg(commands::c2s_broadcast);
	client1.send(msg);

	server.wait_for_msg();
	assert(server.client_said_broadcast == true);

	client1.wait_for_msg();
	assert(client1.server_said_broadcast == true);

	client2.wait_for_msg();
	assert(client2.server_said_broadcast == true);
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		service_connect();
		service_client_hello();
		service_echo();
		service_ping_pong();
		service_broadcast();
	}
	else {
		switch (std::stoi(argv[1])) {
			case 0: service_connect();			break;
			case 1: service_client_hello();		break;
			case 2: service_echo();				break;
			case 3: service_ping_pong();		break;
			case 4: service_broadcast();		break;
			default: break;
		}
	}

	return 0;
}