#include "libnetwrk.hpp"
#include "commands.hpp"

#include <iostream>

using namespace libnetwrk::net::tcp;
using namespace libnetwrk::net;

class tcp_echo_client : public tcp_client<commands> {
	public:
		tcp_echo_client() : tcp_client() {}

	void on_message(message<commands>& msg) override {
		switch (msg.m_head.m_command) {
			case commands::s2c_echo: {
				std::string text;
				msg >> text;
				LIBNETWRK_INFO("%s", text.c_str());
				break;
			}
			default: break;
		}
	}
};

int main(int argc, char* argv[]) {
	tcp_echo_client client;
	client.connect("127.0.0.1", 21205);
	client.process_messages_async();

	while (true) {
		std::string input;
		std::getline(std::cin, input);

		if (input == "q") break;
		
		tcp_echo_client::message_t msg(commands::c2s_echo);
		msg << input;
		client.send(msg);
	}

	client.stop();

	return 0;
}