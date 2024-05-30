#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include <libnetwrk.hpp>
#include <gtest/gtest.h>

enum class commands : int {
    hello
};

struct service_desc {
    using command_t = commands;
    using storage_t = libnetwrk::nothing;
};

TEST(tcp_service, create) {
    ASSERT_NO_THROW(libnetwrk::tcp::tcp_service<service_desc> service);

    libnetwrk::tcp::tcp_service<service_desc> service;
    EXPECT_FALSE(service.is_running());
}

TEST(tcp_service, start_ip) {
    libnetwrk::tcp::tcp_service<service_desc> service;
    EXPECT_TRUE(service.start("127.0.0.1", 0));
    EXPECT_TRUE(service.is_running());
}

TEST(tcp_service, start_twice) {
    libnetwrk::tcp::tcp_service<service_desc> service;
    EXPECT_TRUE(service.start("127.0.0.1", 0));
    EXPECT_FALSE(service.start("127.0.0.1", 0));
    EXPECT_TRUE(service.is_running());
}

TEST(tcp_service, stop) {
    libnetwrk::tcp::tcp_service<service_desc> service;
    service.start("127.0.0.1", 0);
    EXPECT_TRUE(service.is_running());
    service.stop();
    EXPECT_FALSE(service.is_running());
}
