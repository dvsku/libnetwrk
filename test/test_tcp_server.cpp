#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include <libnetwrk.hpp>
#include <gtest/gtest.h>

enum class commands : int {
    hello
};

TEST(tcp_server, create) {
    ASSERT_NO_THROW(libnetwrk::tcp::tcp_server<commands> server);

    libnetwrk::tcp::tcp_server<commands> server;
    EXPECT_FALSE(server.running());
}

TEST(tcp_server, start_ip) {
    libnetwrk::tcp::tcp_server<commands> server;
    EXPECT_TRUE(server.start("127.0.0.1", 21205));
    EXPECT_TRUE(server.running());
}

TEST(tcp_server, start_twice) {
    libnetwrk::tcp::tcp_server<commands> server;
    EXPECT_TRUE(server.start("127.0.0.1", 21205));
    EXPECT_FALSE(server.start("127.0.0.1", 21205));
    EXPECT_TRUE(server.running());
}

TEST(tcp_server, stop) {
    libnetwrk::tcp::tcp_server<commands> server;
    server.start("127.0.0.1", 21205);
    EXPECT_TRUE(server.running());
    server.stop();
    EXPECT_FALSE(server.running());
}
