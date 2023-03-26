#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include "libnetwrk.hpp"
#include "utilities_assert.hpp"

using namespace libnetwrk::net::common;

void buffer_create() {
	buffer b1;
	ASSERT(b1.size() == 0);

	std::vector<uint8_t> v({ 1, 2, 3, 4, 5, 6 });
	buffer b2(v.begin(), v.end());
	ASSERT(b2.size() == 6);
}

void buffer_get_range() {
	buffer b;
	b << uint8_t(1) << uint8_t(2) << uint8_t(3) << uint8_t(4) << uint8_t(5);
	
	buffer range = b.get_range(3);
	ASSERT(range.size() == 3);
	ASSERT(range[0] == 1);
	ASSERT(range[1] == 2);
	ASSERT(range[2] == 3);

	uint8_t c1 = 0, c2 = 0;
	b.get_range(&c1, sizeof(uint8_t));
	b.get_range(&c2, sizeof(uint8_t));
	ASSERT(c1 == 4);
	ASSERT(c2 == 5);
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

	ASSERT(b2.size() == 3);
	ASSERT(b2[0] == 1);
	ASSERT(b2[1] == 2);
	ASSERT(b2[2] == 3);

	uint8_t ui = 24;
	b2.push_back(&ui, sizeof(uint8_t));
	ASSERT(b2[3] == 24);
}

void buffer_push_at() {
	buffer b1, b2, b3, b4;

	uint32_t i1 = 321, i2 = 8543;

	b1.push_at(&i1, sizeof(i1), 0);
	b1.push_at(&i2, sizeof(i2), 0);

	uint32_t* pi1 = (uint32_t*)(b1.data());
	uint32_t* pi2 = (uint32_t*)(b1.data() + sizeof(uint32_t));

	ASSERT(i1 == *pi2);
	ASSERT(i2 == *pi1);

	b2.push_back(&i1, sizeof(uint32_t));
	b3.push_back(&i2, sizeof(uint32_t));

	b4.push_at(b2, 0);
	b4.push_at(b3, 0);

	pi1 = (uint32_t*)(b4.data());
	pi2 = (uint32_t*)(b4.data() + sizeof(uint32_t));

	ASSERT(i1 == *pi2);
	ASSERT(i2 == *pi1);
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		buffer_create();
		buffer_get_range();
		buffer_push_back_buffer();
		buffer_push_at();
		buffer_get_range_exception();
	}
	else {
		switch (std::stoi(argv[1])) {
			case 0: buffer_create();				break;
			case 1: buffer_get_range();				break;
			case 2: buffer_push_back_buffer();		break;
			case 3: buffer_push_at();				break;
			case 4: buffer_get_range_exception();	break;
			default: break;
		}
	}

	return 0;
}