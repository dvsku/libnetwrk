#include "libnetwrk.hpp"

#include <cassert>

using namespace libnetwrk::net;

void message_create() {
	message<int> msg(1);
	
	assert(msg.m_head.m_command == 1);
	assert(msg.m_head.m_data_len == 0);
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		message_create();
	}
	else {
		switch (std::stoi(argv[1])) {
			case 0: message_create();			break;
			default: break;
		}
	}

	return 0;
}