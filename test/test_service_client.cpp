#include <libnetwrk.hpp>
#include <gtest/gtest.h>

using namespace libnetwrk;
using namespace libnetwrk::tcp;

enum class commands : unsigned int {
    c2s_hello,
};

struct service_desc {
    using command_t = commands;
    using storage_t = libnetwrk::nothing;
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

TEST(service_client, storage) {
    struct no_storage {};

    struct has_not_exactly_storage {
        using not_exactly_storage_t = no_storage;
    };

    struct has_storage {
        using storage_t = no_storage;
    };

    ASSERT_FALSE(desc_has_storage_type<no_storage>);
    ASSERT_FALSE(desc_has_storage_type<has_not_exactly_storage>);
    ASSERT_TRUE(desc_has_storage_type<has_storage>);
}

TEST(service_client, auth_timeout) {
    test_service service;
    service.get_settings().gc_freq_sec       = 1;
    service.get_settings().auth_deadline_sec = 4;
    service.start("127.0.0.1", 0);

    tcp_client<service_desc> client;
    client.connect("127.0.0.1", service.get_port());

    service.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    while (service.connections() != 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    EXPECT_FALSE(client.is_connected());
    EXPECT_TRUE(service.client_connected);
    EXPECT_TRUE(service.client_disconnected);
    EXPECT_TRUE(service.dc_code == libnetwrk::disconnect_code::authentication_failed);
}