#include "libnetwrk.hpp"

using namespace libnetwrk::net::common;

struct custom_struct : public serializable {
	uint8_t a		= 1;
	uint16_t b		= 1337;
	std::string c	= "sErIaLiZaBle TesT";
	uint32_t d		= 420;
	uint64_t e		= 69;
	float f			= 15.34;
	double g		= 8342.2;
	bool h			= false;

	BUFFER_U8 serialize() const override {

	}

	void deserialize(BUFFER_U8 serialized) override {

	}

	size_t size() const override {

	}

	bool equals(const custom_struct& obj) {
		return a == obj.a && b == obj.b && c == obj.c && d == obj.d && 
			e == obj.e && f == obj.f && g == obj.g && h == obj.h;
	}
};

void serialize_deserialize_custom_struct() {
	custom_struct cs;
}

int main(int argc, char* argv[]) {
	if (argc != 2) return -1;

	switch (std::stoi(argv[1])) {
		case 0: serialize_deserialize_custom_struct();			break;
		default: break;
	}

	return 0;
}