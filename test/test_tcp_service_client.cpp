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
    using command_t = commands;
    using storage_t = libnetwrk::nothing;
};

class test_service : public tcp_service<service_desc> {
    public:
        test_service() : tcp_service() {
            set_message_callback([this](auto command, auto message) {
                ev_message(command, message);
            });
        }

        bool client_said_hello = false;
        bool client_said_echo = false;
        bool client_said_broadcast = false;
        std::string ping = "";
        
        void ev_message(command_t command, owned_message_t* msg) {
            message_t response;
            switch (command) {
                case commands::c2s_hello:
                    client_said_hello = true;
                    break;
                case commands::c2s_echo:
                    client_said_echo = true;
                    response.set_command(commands::s2c_echo);
                    msg->sender->send(response);
                    break;
                case commands::c2s_ping:
                    msg->msg >> ping;
                    response.set_command(commands::s2c_pong);
                    response << std::string("pOnG");
                    msg->sender->send(response);
                    break;
                case commands::c2s_broadcast:
                    client_said_broadcast = true;
                    response.set_command(commands::s2c_broadcast);
                    send_all(response);
                    break;
                case commands::c2s_send_sync_success:
                    response.set_command(commands::s2c_send_sync_success);
                    response << std::string("success");
                    msg->sender->send(response);
                    break;
                case commands::c2s_send_sync_fail:
                    response.set_command(commands::s2c_send_sync_fail);
                    response << std::string("fail");
                    std::this_thread::sleep_for(std::chrono::milliseconds(5500));
                    msg->sender->send(response);
                    break;
                default:
                    break;
            }
        }

        bool is_correct_id(uint32_t index, uint64_t id) {
            if (index > m_comp_connection.connections.size() - 1) return false;

            auto front = m_comp_connection.connections.begin();
            std::advance(front, index);
            return (*front)->get_id() == id;
        }
};

class test_client : public tcp_client<service_desc> {
    public:
        test_client() : tcp_client() {
            set_message_callback([this](auto command, auto message) {
                ev_message(command, message);
            });
        }

        bool service_said_echo = false;
        bool service_said_broadcast = false;
        std::string pong = "";

        void ev_message(command_t command, owned_message_t* msg) {
            switch (command) {
                case commands::s2c_echo:
                    service_said_echo = true;
                    break;
                case commands::s2c_pong:
                    msg->msg >> pong;
                    break;
                case commands::s2c_broadcast:
                    service_said_broadcast = true;
                    break;

                default:
                    break;
            }
        }
};

class test_service_pp : public tcp_service<service_desc> {
public:
    test_service_pp() : tcp_service() {
        set_message_callback([this](auto command, auto message) {
            ev_message(command, message);
        });

        set_pre_process_message_callback([this](auto buffer) {
            pre_process_message(buffer);
        });

        set_post_process_message_callback([this](auto buffer) {
            post_process_message(buffer);
        });
    }

    std::string ping = "";

    void ev_message(command_t command, owned_message_t* msg) {
        message_t response;
        switch (command) {
            case commands::c2s_ping:
                msg->msg >> ping;
                response.set_command(commands::s2c_pong);
                response << std::string("pOnG");
                msg->sender->send(response);
                break;
            default:
                break;
        }
    }

protected:
    void pre_process_message(dynamic_buffer* buffer) {
        for (uint8_t& byte : buffer->underlying()) {
            byte ^= 69;
        }

        buffer->underlying().push_back(155);
    }

    void post_process_message(dynamic_buffer* buffer) {
        buffer->underlying().resize(buffer->size() - 1);

        for (uint8_t& byte : buffer->underlying()) {
            byte ^= 69;
        }
    }
};

class test_client_pp : public tcp_client<service_desc> {
public:
    test_client_pp() : tcp_client() {
        set_message_callback([this](auto command, auto message) {
            ev_message(command, message);
        });

        set_pre_process_message_callback([this](auto buffer) {
            pre_process_message(buffer);
        });

        set_post_process_message_callback([this](auto buffer) {
            post_process_message(buffer);
        });
    }

    std::string pong = "";

    void ev_message(command_t command, owned_message_t* msg) {
        switch (command) {
            case commands::s2c_pong:
                msg->msg >> pong;
                break;
            default:
                break;
        }
    }

protected:
    void pre_process_message(dynamic_buffer* buffer) {
        for (uint8_t& byte : buffer->underlying()) {
            byte ^= 69;
        }

        buffer->underlying().push_back(155);
    }

    void post_process_message(dynamic_buffer* buffer) {
        buffer->underlying().resize(buffer->size() - 1);

        for (uint8_t& byte : buffer->underlying()) {
            byte ^= 69;
        }
    }
};

TEST(tcp_service_client, connect) {
    {
        test_service service;
        EXPECT_TRUE(service.start("127.0.0.1", 0));

        test_client client;
        EXPECT_TRUE(client.connect("127.0.0.1", service.get_port()));
    }

    {
        test_service service;
        EXPECT_TRUE(service.start("localhost", 0));

        test_client client;
        EXPECT_TRUE(client.connect("localhost", service.get_port()));
    }
}

TEST(tcp_service_client, hello) {
    test_service service;
    service.start("127.0.0.1", 0);

    test_client client;
    client.connect("127.0.0.1", service.get_port());

    test_client::message_t msg(commands::c2s_hello);
    client.send(msg);

    service.process_messages_async();
    client.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    EXPECT_TRUE(service.client_said_hello == true);
}

TEST(tcp_service_client, echo) {
    test_service service;
    service.start("127.0.0.1", 0);

    test_client client;
    client.connect("127.0.0.1", service.get_port());

    test_client::message_t msg(commands::c2s_echo);
    client.send(msg);

    service.process_messages_async();
    client.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    EXPECT_TRUE(service.client_said_echo == true);
    EXPECT_TRUE(client.service_said_echo == true);
}

TEST(tcp_service_client, ping_pong) {
    test_service service;
    service.start("127.0.0.1", 0);

    test_client client;
    client.connect("127.0.0.1", service.get_port());

    test_client::message_t msg(commands::c2s_ping);
    msg << std::string("PiNg");
    client.send(msg);

    service.process_messages_async();
    client.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    EXPECT_TRUE(service.ping == "PiNg");
    EXPECT_TRUE(client.pong == "pOnG");
}

TEST(tcp_service_client, broadcast) {
    test_service service;
    service.start("127.0.0.1", 0);

    test_client client1;
    EXPECT_TRUE(client1.connect("127.0.0.1", service.get_port()) == true);

    test_client client2;
    EXPECT_TRUE(client2.connect("127.0.0.1", service.get_port()) == true);

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    EXPECT_TRUE(service.is_correct_id(0, 1));
    EXPECT_TRUE(service.is_correct_id(1, 2));

    test_client::message_t msg(commands::c2s_broadcast);
    client1.send(msg);

    service.process_messages_async();
    client1.process_messages_async();
    client2.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    EXPECT_TRUE(service.client_said_broadcast  == true);
    EXPECT_TRUE(client1.service_said_broadcast == true);
    EXPECT_TRUE(client2.service_said_broadcast == true);
}

TEST(tcp_service_client, ping_pong_pre_post_process) {
    test_service_pp service;
    service.start("127.0.0.1", 0);

    test_client_pp client;
    client.connect("127.0.0.1", service.get_port());

    test_client_pp::message_t msg(commands::c2s_ping);
    msg << std::string("PiNg");
    client.send(msg);

    service.process_messages_async();
    client.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    EXPECT_TRUE(service.ping == "PiNg");
    EXPECT_TRUE(client.pong == "pOnG");
}

TEST(tcp_service_client, two_clients_out_of_order) {
    test_service service;
    service.start("127.0.0.1", 0);
    service.process_messages_async();

    test_client client1;
    EXPECT_TRUE(client1.connect("127.0.0.1", service.get_port()) == true);
    client1.process_messages_async();

    test_client client2;
    EXPECT_TRUE(client2.connect("127.0.0.1", service.get_port()) == true);
    client2.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    {
        test_client::message_t msg(commands::c2s_ping);
        msg << std::string("PiNg");
        client2.send(msg);
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    {
        test_client::message_t msg(commands::c2s_ping);
        msg << std::string("PiNg");
        client1.send(msg);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    EXPECT_TRUE(client1.pong == "pOnG");
    EXPECT_TRUE(client2.pong == "pOnG");
}

TEST(tcp_service_client, send_keep_message) {
    test_service service;
    service.start("127.0.0.1", 0);
    service.process_messages_async();

    test_client client;
    EXPECT_TRUE(client.connect("127.0.0.1", service.get_port()) == true);
    client.process_messages_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    {
        test_service::message_t message(commands::s2c_pong);
        message << std::string("pOnG");
        service.send_all(message, libnetwrk::send_flags::keep_message);

        EXPECT_TRUE(message.command()   == commands::s2c_pong);
        EXPECT_TRUE(message.data.size() != 0);
    }

    {
        test_client::message_t message(commands::c2s_ping);
        message << std::string("PiNg");
        client.send(message, libnetwrk::send_flags::keep_message);

        EXPECT_TRUE(message.command()   == commands::c2s_ping);
        EXPECT_TRUE(message.data.size() != 0);
    }
    
    {
        test_service::message_t message(commands::s2c_pong);
        message << std::string("pOnG");
        service.send_all(message, libnetwrk::send_flags::none);

        EXPECT_TRUE(message.command()   != commands::s2c_pong);
        EXPECT_TRUE(message.data.size() == 0);
    }

    {
        test_client::message_t message(commands::c2s_ping);
        message << std::string("PiNg");
        client.send(message, libnetwrk::send_flags::none);

        EXPECT_TRUE(message.command()   != commands::c2s_ping);
        EXPECT_TRUE(message.data.size() == 0);
    }
}