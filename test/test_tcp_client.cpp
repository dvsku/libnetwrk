#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include "libnetwrk.hpp"
#include "utilities_assert.hpp"

using namespace libnetwrk::net::tcp;

enum class commands : int {
    hello,
};

void tcp_client_create() {
    ASSERT_NOT_THROWS_CTOR(tcp_client<commands> client);
    ASSERT(!client.connected());
}

void tcp_client_connect_fail() {
    ASSERT_NOT_THROWS_CTOR(tcp_client<commands> client);
    ASSERT(!client.connect("127.0.0.1", 21205));
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        tcp_client_create();
        tcp_client_connect_fail();
    }
    else {
        switch (std::stoi(argv[1])) {
            case 0: tcp_client_create();            break;
            case 1: tcp_client_connect_fail();        break;
            default: break;
        }
    }

    return 0;
}