#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include <libnetwrk.hpp>
#include <gtest/gtest.h>

#include <thread>
#include <chrono>

using namespace libnetwrk::tcp;
using namespace libnetwrk;

enum class commands : unsigned int {
    c2s_hello,
    c2s_echo,
    s2c_echo,
    c2s_ping,
    s2c_pong,
    c2s_broadcast,
    s2c_broadcast,
    c2s_send_sync_success,
    s2c_send_sync_success,
    c2s_send_sync_fail,
    s2c_send_sync_fail
};

struct service_desc {
    using command_t   = commands;
    using serialize_t = libnetwrk::bin_serialize;
    using storage_t   = libnetwrk::nothing;
};

class test_service : public tcp_server<service_desc> {
    public:
        test_service() : tcp_server() {}

        bool client_said_hello = false;
        bool client_said_echo = false;
        bool client_said_broadcast = false;
        std::string ping = "";
        
        void ev_message(owned_message_t& msg) override {
            message_t response;
            switch (msg.msg.command()) {
                case commands::c2s_hello:
                    client_said_hello = true;
                    break;
                case commands::c2s_echo:
                    client_said_echo = true;
                    response.set_command(commands::s2c_echo);
                    msg.sender->send(response);
                    break;
                case commands::c2s_ping:
                    msg.msg >> ping;
                    response.set_command(commands::s2c_pong);
                    response << std::string("pOnG");
                    msg.sender->send(response);
                    break;
                case commands::c2s_broadcast:
                    client_said_broadcast = true;
                    response.set_command(commands::s2c_broadcast);
                    send_all(response);
                    break;
                case commands::c2s_send_sync_success:
                    response.set_command(commands::s2c_send_sync_success);
                    response << std::string("success");
                    msg.sender->send(response);
                    break;
                case commands::c2s_send_sync_fail:
                    response.set_command(commands::s2c_send_sync_fail);
                    response << std::string("fail");
                    std::this_thread::sleep_for(std::chrono::milliseconds(5500));
                    msg.sender->send(response);
                    break;
                default:
                    break;
            }
        }

        bool is_correct_id(uint32_t index, uint64_t id) {
            if (index > m_connections.size() - 1) return false;

            auto front = m_connections.begin();
            std::advance(front, index);
            return (*front)->id() == id;
        }
};

class test_client : public tcp_client<service_desc> {
    public:
        test_client() : tcp_client() {}

        bool server_said_echo = false;
        bool server_said_broadcast = false;
        std::string pong = "";

        void ev_message(owned_message_t& msg) override {
            switch (msg.msg.command()) {
                case commands::s2c_echo:
                    server_said_echo = true;
                    break;
                case commands::s2c_pong:
                    msg.msg >> pong;
                    break;
                case commands::s2c_broadcast:
                    server_said_broadcast = true;
                    break;

                default:
                    break;
            }
        }
};

TEST(tcp_server_client, connect) {
    test_service server;
    server.start("127.0.0.1", 21205);

    test_client client;
    bool connected = client.connect("127.0.0.1", 21205);

    EXPECT_TRUE(connected == true);
}

TEST(tcp_server_client, hello) {
    test_service server;
    server.start("127.0.0.1", 21205);

    test_client client;
    client.connect("127.0.0.1", 21205);

    test_client::message_t msg(commands::c2s_hello);
    client.send(msg);

    server.process_messages_async();
    client.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    EXPECT_TRUE(server.client_said_hello == true);
}

TEST(tcp_server_client, echo) {
    test_service server;
    server.start("127.0.0.1", 21205);

    test_client client;
    client.connect("127.0.0.1", 21205);

    test_client::message_t msg(commands::c2s_echo);
    client.send(msg);

    server.process_messages_async();
    client.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    EXPECT_TRUE(server.client_said_echo == true);
    EXPECT_TRUE(client.server_said_echo == true);
}

TEST(tcp_server_client, ping_pong) {
    test_service server;
    server.start("127.0.0.1", 21205);

    test_client client;
    client.connect("127.0.0.1", 21205);

    test_client::message_t msg(commands::c2s_ping);
    msg << std::string("PiNg");
    client.send(msg);

    server.process_messages_async();
    client.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    EXPECT_TRUE(server.ping == "PiNg");
    EXPECT_TRUE(client.pong == "pOnG");
}

TEST(tcp_server_client, broadcast) {
    test_service server;
    server.start("127.0.0.1", 21205);

    test_client client1;
    EXPECT_TRUE(client1.connect("127.0.0.1", 21205) == true);

    test_client client2;
    EXPECT_TRUE(client2.connect("127.0.0.1", 21205) == true);

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    EXPECT_TRUE(server.is_correct_id(0, 1));
    EXPECT_TRUE(server.is_correct_id(1, 2));

    test_client::message_t msg(commands::c2s_broadcast);
    client1.send(msg);

    server.process_messages_async();
    client1.process_messages_async();
    client2.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    EXPECT_TRUE(server.client_said_broadcast  == true);
    EXPECT_TRUE(client1.server_said_broadcast == true);
    EXPECT_TRUE(client2.server_said_broadcast == true);
}
