#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include <libnetwrk.hpp>
#include <gtest/gtest.h>

using namespace libnetwrk::tcp;
using namespace libnetwrk;

enum class commands : unsigned int {
    c2s_msg1,
    c2s_msg2,
    c2s_msg3,
    c2s_msg4,

    s2c_msg1,
    s2c_msg2,
    s2c_msg3,
    s2c_msg4,
};

struct service_desc {
    using command_t   = commands;
    using serialize_t = libnetwrk::bin_serialize;
    using storage_t   = libnetwrk::nothing;
};

class test_service : public tcp_service<service_desc> {
public:
    test_service() : tcp_service() {}

    void ev_message(owned_message_t& msg) override {
        message_t response;
        
        std::string received;
        msg.msg >> received;

        switch (msg.msg.command()) {
            case commands::c2s_msg1: {
                EXPECT_TRUE(received == "request_1");

                response.set_command(commands::s2c_msg1);
                response << "response_1";
                msg.sender->send(response);
                break;
            }
            case commands::c2s_msg2: {
                EXPECT_TRUE(received == "request_2");

                response.set_command(commands::s2c_msg2);
                response << "response_2";
                msg.sender->send(response);
                break;
            }
            case commands::c2s_msg3: {
                EXPECT_TRUE(received == "request_3");

                response.set_command(commands::s2c_msg3);
                response << "response_3";
                msg.sender->send(response);
                break;
            }
            case commands::c2s_msg4: {
                EXPECT_TRUE(received == "request_4");

                response.set_command(commands::s2c_msg4);
                response << "response_4";
                msg.sender->send(response);

                break;
            }
            default: break;
        }
    }
};

class test_client : public tcp_client<service_desc> {
public:
    test_client() : tcp_client() {
        set_message_callback([this](auto command, auto message) {
            ev_message(command, message);
        });
    }

    void ev_message(command_t command, owned_message_t* msg) {
        message_t response;

        std::string received;
        msg->msg >> received;

        switch (command) {
            case commands::s2c_msg1: {
                EXPECT_TRUE(received == "response_1");

                response.set_command(commands::c2s_msg2);
                response << "request_2";
                send(response);
                break;
            }
            case commands::s2c_msg2: {
                EXPECT_TRUE(received == "response_2");

                response.set_command(commands::c2s_msg3);
                response << "request_3";
                send(response);
                break;
            }
            case commands::s2c_msg3: {
                EXPECT_TRUE(received == "response_3");

                response.set_command(commands::c2s_msg4);
                response << "request_4";
                send(response);
                break;
            }
            case commands::s2c_msg4: {
                EXPECT_TRUE(received == "response_4");

                disconnect();
                break;
            }
            default: break;
        }
    }
};

TEST(tcp_talk, talking) {
    test_service service;
    service.start("127.0.0.1", 21205);

    test_client client;
    client.connect("127.0.0.1", 21205);

    test_client::message_t msg(test_client::command_t::c2s_msg1);
    msg << "request_1";
    client.send(msg);

    while (client.is_connected()) {
        service.process_message();
        client.process_message();
    }
}
