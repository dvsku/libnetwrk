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
    bool                       client_connected    = false;
    bool                       client_disconnected = false;
    libnetwrk::disconnect_code dc_code             = libnetwrk::disconnect_code::unspecified;

    test_service() : tcp_service() {
        set_connect_callback([this](auto) {
            client_connected = true;
        });

        set_disconnect_callback([this](auto, auto code) {
            client_disconnected = true;
            dc_code = code;
        });
    }

    size_t connections() {
        return m_comp_connection.connections.size();
    }
};

TEST(service_client, auth_timeout) {
    test_service service;
    service.get_settings().gc_freq_sec       = 1;
    service.get_settings().auth_deadline_sec = 4;
    service.start("127.0.0.1", 0);

    tcp_client<service_desc> client;
    client.connect("127.0.0.1", service.get_port());

    service.process_messages_async();

    while (service.connections() != 0 || client.is_connected()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    EXPECT_TRUE(service.client_connected);
    EXPECT_TRUE(service.client_disconnected);
    EXPECT_TRUE(service.dc_code == libnetwrk::disconnect_code::authentication_failed);
}