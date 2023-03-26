#include "libnetwrk.hpp"

#include <cassert>

using namespace libnetwrk::net;

void message_create() {
	message<int> m1(1);
	
	assert(m1.m_head.m_command == 1);
	assert(m1.m_head.m_data_len == 0);

	m1 << 15;

	message<int> m2(m1);

	assert(m2.m_head.m_command == 1);
	assert(m2.m_head.m_data_len == sizeof(int));
	assert(!m2.m_data.empty());

	message<int> m3(std::move(m2));
	assert(m3.m_head.m_command == 1);
	assert(m3.m_head.m_data_len == sizeof(int));
	assert(!m3.m_data.empty());
	
	assert(m2.m_head.m_command == int{});			// Ignore warnings about use of moved from object
	assert(m2.m_head.m_data_len == 0);				// Ignore warnings about use of moved from object
	assert(m2.m_data.empty());						// Ignore warnings about use of moved from object
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