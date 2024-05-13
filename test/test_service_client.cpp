#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include <libnetwrk.hpp>
#include <gtest/gtest.h>

using namespace libnetwrk;
using namespace libnetwrk::tcp;

enum class commands : unsigned int {
    c2s_hello,
};

struct service_desc {
    using command_t   = commands;
    using serialize_t = libnetwrk::bin_serialize;
    using storage_t   = libnetwrk::nothing;
};

class test_service : public tcp_service<service_desc> {
public:
    test_service() : tcp_service() {}

    size_t connections() {
        return m_connections.size();
    }
};

TEST(service_client, auth_timeout) {
    test_service service;
    service.gc_freq_sec = 1;
    service.start("127.0.0.1", 21205);

    tcp_client<service_desc> client;
    client.connect("127.0.0.1", 21205);

    service.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(7500));

    EXPECT_TRUE(service.connections() == 1);

    std::this_thread::sleep_for(std::chrono::milliseconds(7500));

    EXPECT_TRUE(service.connections() == 0);
}