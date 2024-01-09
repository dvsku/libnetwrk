#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include "libnetwrk.hpp"
#include "utilities_assert.hpp"

using namespace libnetwrk;

struct simple_struct {
    uint8_t a        = 1;
    uint16_t b        = 1337;
    uint32_t c        = 420;
    uint64_t d        = 69;
    float e            = 15.34f;
    double f        = 8342.2;
    bool g            = false;

    template<typename Tserialize>
    void serialize(buffer<Tserialize>& buffer) const {
        buffer << a << b << c << d << e << f << g;
    }

    template<typename Tserialize>
    void deserialize(buffer<Tserialize>& buffer) {
        buffer >> a >> b >> c >> d >> e >> f >> g;
    }

    bool equals(const simple_struct& obj) {
        return a == obj.a && b == obj.b && c == obj.c && d == obj.d && 
            e == obj.e && f == obj.f && g == obj.g;
    }
};

struct derived_struct : simple_struct {
    uint32_t h     = 365464;
    char     i[32] = "AsagfiOUSFHfdsifudsf";

    template<typename Tserialize>
    void serialize(buffer<Tserialize>& buffer) const {
        simple_struct::serialize<Tserialize>(buffer);
        buffer << h << i;
    }

    template<typename Tserialize>
    void deserialize(buffer<Tserialize>& buffer) {
        simple_struct::deserialize<Tserialize>(buffer);
        buffer >> h >> i;
    }

    bool equals(const derived_struct& obj) {
        return simple_struct::equals(obj) && h == obj.h && (strcmp(i, obj.i) == 0);
    }
};

static void serialize_deserialize_simple_struct() {
    buffer<bin_serialize> buff;
    simple_struct ss1{}, ss2{};

    ss1.serialize(buff);
    ss2.deserialize(buff);
    
    ASSERT(ss1.equals(ss2));
}

static void serialize_deserialize_derived_struct() {
    buffer<bin_serialize> buff;

    derived_struct ss1{}, ss2{};
    ss2.a = 5;
    ss2.h = 777;

    ASSERT(!ss1.equals(ss2));

    ss1.serialize(buff);
    ss2.deserialize(buff);

    ASSERT(ss1.equals(ss2));
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        serialize_deserialize_simple_struct();
        serialize_deserialize_derived_struct();
    }
    else {
        switch (std::stoi(argv[1])) {
            case 0: serialize_deserialize_simple_struct();  break;
            case 1: serialize_deserialize_derived_struct(); break;
            default: break;
        }
    }

    return 0;
}