#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include "libnetwrk.hpp"
#include "utilities_assert.hpp"

using namespace libnetwrk;

struct simple_struct : public serializable<bin_serialize> {
    uint8_t a        = 1;
    uint16_t b        = 1337;
    uint32_t c        = 420;
    uint64_t d        = 69;
    float e            = 15.34f;
    double f        = 8342.2;
    bool g            = false;

    void serialize(buffer_t& buffer) const override {
        buffer << a << b << c << d << e << f << g;
    }

    void deserialize(buffer_t& serialized) override {
        serialized >> a >> b >> c >> d >> e >> f >> g;
    }

    bool equals(const simple_struct& obj) {
        return a == obj.a && b == obj.b && c == obj.c && d == obj.d && 
            e == obj.e && f == obj.f && g == obj.g;
    }
};

void serialize_deserialize_simple_struct() {
    buffer<bin_serialize> buff;
    simple_struct ss1{}, ss2{};

    ss1.serialize(buff);
    ss2.deserialize(buff);
    
    ASSERT(ss1.equals(ss2));
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        serialize_deserialize_simple_struct();
    }
    else {
        switch (std::stoi(argv[1])) {
            case 0: serialize_deserialize_simple_struct(); break;
            default: break;
        }
    }

    return 0;
}