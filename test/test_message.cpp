#include "net/common/message.hpp"

#include <cassert>

using namespace libnetwrk::net::common;

void message_create() {
	message<int> msg(1);
	
	assert(msg.m_head.m_command == 1);
	assert(msg.m_head.m_data_len == 0);
}

void run_all_tests() {
	message_create();
}

int main(int argc, char* argv[]) {
	run_all_tests();
	return 0;
}