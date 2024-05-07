#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include <libnetwrk.hpp>
#include <gtest/gtest.h>

enum class commands : int {
    hello
};

struct service_desc {
    using command_t   = commands;
    using serialize_t = libnetwrk::bin_serialize;
    using storage_t   = libnetwrk::nothing;
};

TEST(tcp_server, create) {
    ASSERT_NO_THROW(libnetwrk::tcp::tcp_server<service_desc> server);

    libnetwrk::tcp::tcp_server<service_desc> server;
    EXPECT_FALSE(server.running());
}

TEST(tcp_server, start_ip) {
    libnetwrk::tcp::tcp_server<service_desc> server;
    EXPECT_TRUE(server.start("127.0.0.1", 21205));
    EXPECT_TRUE(server.running());
}

TEST(tcp_server, start_twice) {
    libnetwrk::tcp::tcp_server<service_desc> server;
    EXPECT_TRUE(server.start("127.0.0.1", 21205));
    EXPECT_FALSE(server.start("127.0.0.1", 21205));
    EXPECT_TRUE(server.running());
}

TEST(tcp_server, stop) {
    libnetwrk::tcp::tcp_server<service_desc> server;
    server.start("127.0.0.1", 21205);
    EXPECT_TRUE(server.running());
    server.stop();
    EXPECT_FALSE(server.running());
}
