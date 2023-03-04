#include "libnetwrk.hpp"

using namespace libnetwrk::net::common;

void buffer_create() {
	buffer buffer;
	assert(buffer.size() == 0);
}

void buffer_get_range() {
	buffer b;
	b.push_back(1);
	b.push_back(2);
	b.push_back(3);
	b.push_back(4);
	b.push_back(5);
	
	buffer range = b.get_range(1, 3);
	assert(range.size() == 3);
	assert(range[0] == 2);
	assert(range[1] == 3);
	assert(range[2] == 4);
}

void buffer_push_back_buffer() {
	buffer b1;
	b1.push_back(1);
	b1.push_back(2);
	b1.push_back(3);

	buffer b2;
	b2.push_back(b1);

	assert(b2.size() == 3);
	assert(b2[0] == 1);
	assert(b2[1] == 2);
	assert(b2[2] == 3);
}

int main(int argc, char* argv[]) {
	if (argc != 2) return -1;

	switch (std::stoi(argv[1])) {
		case 0: buffer_create();			break;
		case 1: buffer_get_range();			break;
		case 2: buffer_push_back_buffer();	break;
		default: break;
	}

	return 0;
}