#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include "libnetwrk.hpp"

#include "utilities_assert.hpp"

using namespace libnetwrk::net;

enum class commands : unsigned int {
    undefined = 0x00,
    hello = 0x01,
};

void message_create() {
    ASSERT_THROWS(message<uint8_t>        m(1));
    ASSERT_THROWS(message<uint16_t>        m(1));
    ASSERT_THROWS(message<uint32_t>        m(1));
    ASSERT_THROWS(message<uint64_t>        m(1));
    ASSERT_THROWS(message<int8_t>        m(-1));
    ASSERT_THROWS(message<int16_t>        m(-1));
    ASSERT_THROWS(message<int32_t>        m(-1));
    ASSERT_THROWS(message<int64_t>        m(-1));
    ASSERT_THROWS(message<float>        m(1.0f));
    ASSERT_THROWS(message<double>        m(1.0));
    ASSERT_THROWS(message<std::string>    m(""));

    ASSERT_NOT_THROWS_CTOR(message<commands> m1(commands::hello));

    ASSERT(m1.m_head.m_command == commands::hello);
    ASSERT(m1.m_head.m_data_len == 0);

    m1 << 15;

    message<commands> m2;
    ASSERT_NOT_THROWS_CTOR(m2 = m1);

    ASSERT(m2.m_head.m_command == commands::hello);
    ASSERT(m2.m_head.m_data_len == sizeof(int));
    ASSERT(!m2.m_data.empty());

    message<commands> m3;
    ASSERT_NOT_THROWS(m3 = std::move(m2));

    ASSERT(m3.m_head.m_command == commands::hello);
    ASSERT(m3.m_head.m_data_len == sizeof(int));
    ASSERT(!m3.m_data.empty());
    
    ASSERT(m2.m_head.m_command == commands{});            // Ignore warnings about use of moved from object
    ASSERT(m2.m_head.m_data_len == 0);                    // Ignore warnings about use of moved from object
    ASSERT(m2.m_data.empty());                            // Ignore warnings about use of moved from object
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        message_create();
    }
    else {
        switch (std::stoi(argv[1])) {
            case 0: message_create();            break;
            default: break;
        }
    }

    return 0;
}