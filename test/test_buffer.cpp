#include "libnetwrk.hpp"
#include "utilities_assert.hpp"

using namespace libnetwrk::net::common;

void buffer_create() {
	buffer buffer;
	assert(buffer.size() == 0);
}

void buffer_get_range() {
	buffer b;
	b << uint8_t(1) << uint8_t(2) << uint8_t(3) << uint8_t(4) << uint8_t(5);
	
	buffer range = b.get_range(3);
	assert(range.size() == 3);
	assert(range[0] == 1);
	assert(range[1] == 2);
	assert(range[2] == 3);
}

void buffer_get_range_exception() {
	buffer b1;
	int i1 = 253, i2 = 0;
	ASSERT_THROWS(b1.get_range(sizeof(int)));
	ASSERT_THROWS(b1.get_range(&i2, sizeof(int)));

	buffer b2; b2 << i1;
	b2.get_range(&i1, sizeof(int));
	ASSERT_THROWS(b2.get_range(sizeof(char)));
}

void buffer_push_back_buffer() {
	buffer b1;
	b1 << uint8_t(1) << uint8_t(2) << uint8_t(3);

	buffer b2;
	b2.push_back(b1);

	assert(b2.size() == 3);
	assert(b2[0] == 1);
	assert(b2[1] == 2);
	assert(b2[2] == 3);
}

void buffer_push_at() {
	buffer b;

	uint32_t i1 = 321, i2 = 8543;

	b.push_at(&i1, sizeof(i1), 0);
	b.push_at(&i2, sizeof(i2), 0);

	uint32_t* pi1 = (uint32_t*)(b.data());
	uint32_t* pi2 = (uint32_t*)(b.data() + sizeof(uint32_t));

	assert(i1 == *pi2);
	assert(i2 == *pi1);
}

int main(int argc, char* argv[]) {
	if (argc != 2) return -1;

	switch (std::stoi(argv[1])) {
		case 0: buffer_create();				break;
		case 1: buffer_get_range();				break;
		case 2: buffer_push_back_buffer();		break;
		case 3: buffer_push_at();				break;
		case 4: buffer_get_range_exception();	break;
		default: break;
	}

	return 0;
}