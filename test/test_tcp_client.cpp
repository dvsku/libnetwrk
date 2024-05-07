#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include <libnetwrk.hpp>
#include <gtest/gtest.h>

enum class commands : int {
    hello,
};

struct service_desc {
    using command_t   = commands;
    using serialize_t = libnetwrk::bin_serialize;
    using storage_t   = libnetwrk::nothing;
};

TEST(tcp_client, create) {
    ASSERT_NO_THROW(libnetwrk::tcp::tcp_client<service_desc> client);

    libnetwrk::tcp::tcp_client<service_desc> client;
    EXPECT_FALSE(client.connected());
}

TEST(tcp_client, connect_fail) {
    libnetwrk::tcp::tcp_client<service_desc> client;
    EXPECT_FALSE(client.connect("127.0.0.1", 21205));
}
