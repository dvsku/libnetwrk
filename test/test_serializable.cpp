#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include "libnetwrk.hpp"
#include "utilities_assert.hpp"

using namespace libnetwrk::net::common;

struct simple_struct : public serializable<binary_serializer> {
	uint8_t a		= 1;
	uint16_t b		= 1337;
	uint32_t c		= 420;
	uint64_t d		= 69;
	float e			= 15.34f;
	double f		= 8342.2;
	bool g			= false;

	buffer_t serialize() const override {
		buffer_t buffer;
		buffer << a << b << c << d << e << f << g;
		return buffer;
	}

	void deserialize(buffer_t serialized) override {
		serialized >> a >> b >> c >> d >> e >> f >> g;
	}

	bool equals(const simple_struct& obj) {
		return a == obj.a && b == obj.b && c == obj.c && d == obj.d && 
			e == obj.e && f == obj.f && g == obj.g;
	}
};

void serialize_deserialize_simple_struct() {
	buffer buff;
	simple_struct ss1{}, ss2{};

	buff = ss1.serialize();
	ss2.deserialize(buff);
	
	ASSERT(ss1.equals(ss2));
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		serialize_deserialize_simple_struct();
	}
	else {
		switch (std::stoi(argv[1])) {
			case 0: serialize_deserialize_simple_struct();			break;
			default: break;
		}
	}

	return 0;
}