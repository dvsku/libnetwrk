#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include "libnetwrk.hpp"
#include "utilities_assert.hpp"

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

class test_service : public tcp_server<commands> {
public:
    test_service() : tcp_server() {}

    void on_message(owned_message_t& msg) override {
        message_t response;
        
        std::string received;
        msg.message >> received;

        switch (msg.message.head.command) {
            case commands::c2s_msg1: {
                ASSERT(received == "request_1");

                response.head.command = commands::s2c_msg1;
                response << "response_1";
                msg.client->send(response);
                break;
            }
            case commands::c2s_msg2: {
                ASSERT(received == "request_2");

                response.head.command = commands::s2c_msg2;
                response << "response_2";
                msg.client->send(response);
                break;
            }
            case commands::c2s_msg3: {
                ASSERT(received == "request_3");

                response.head.command = commands::s2c_msg3;
                response << "response_3";
                msg.client->send(response);
                break;
            }
            case commands::c2s_msg4: {
                ASSERT(received == "request_4");

                response.head.command = commands::s2c_msg4;
                response << "response_4";
                msg.client->send(response);

                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                stop();

                break;
            }
            default: break;
        }
    }
};

class test_client : public tcp_client<commands> {
public:
    test_client() : tcp_client() {}

    void on_message(message_t& msg) override {
        message_t response;

        std::string received;
        msg >> received;

        switch (msg.head.command) {
            case commands::s2c_msg1: {
                ASSERT(received == "response_1");

                response.head.command = commands::c2s_msg2;
                response << "request_2";
                send(response);
                break;
            }
            case commands::s2c_msg2: {
                ASSERT(received == "response_2");

                response.head.command = commands::c2s_msg3;
                response << "request_3";
                send(response);
                break;
            }
            case commands::s2c_msg3: {
                ASSERT(received == "response_3");

                response.head.command = commands::c2s_msg4;
                response << "request_4";
                send(response);
                break;
            }
            case commands::s2c_msg4: {
                ASSERT(received == "response_4");

                disconnect();
                break;
            }
            default: break;
        }
    }
};

static void tcp_talk() {
    test_service server;
    server.start("127.0.0.1", 21205);

    test_client client;
    client.connect("127.0.0.1", 21205);

    test_client::message_t msg(test_client::command_t::c2s_msg1);
    msg << "request_1";
    client.send(msg);

    while (server.running() && client.connected()) {
        server.process_message();
        client.process_message();
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        tcp_talk();
    }
    else {
        switch (std::stoi(argv[1])) {
            case 0: tcp_talk(); break;
            default:            break;
        }
    }

    return 0;
}