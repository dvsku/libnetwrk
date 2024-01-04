#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include "libnetwrk.hpp"
#include "utilities_assert.hpp"

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

class test_service : public tcp_server<commands> {
    public:
        test_service() : tcp_server() {}

        bool client_said_hello = false;
        bool client_said_echo = false;
        bool client_said_broadcast = false;
        std::string ping = "";
        
        void on_message(owned_message_t& msg) override {
            message_t response;
            switch (msg.message.head.command) {
                case commands::c2s_hello:
                    client_said_hello = true;
                    break;
                case commands::c2s_echo:
                    client_said_echo = true;
                    response.head.command = commands::s2c_echo;
                    msg.client->send(response);
                    break;
                case commands::c2s_ping:
                    msg.message >> ping;
                    response.head.command = commands::s2c_pong;
                    response << std::string("pOnG");
                    msg.client->send(response);
                    break;
                case commands::c2s_broadcast:
                    client_said_broadcast = true;
                    response.head.command = commands::s2c_broadcast;
                    send_all(response);
                    break;
                case commands::c2s_send_sync_success:
                    response.head.command = commands::s2c_send_sync_success;
                    response << std::string("success");
                    msg.client->send(response);
                    break;
                case commands::c2s_send_sync_fail:
                    response.head.command = commands::s2c_send_sync_fail;
                    response << std::string("fail");
                    std::this_thread::sleep_for(std::chrono::milliseconds(5500));
                    msg.client->send(response);
                    break;
                default:
                    break;
            }
        }

        void wait_for_msg(const int timeout = 30) {
            int tries = 0;
            while (tries < timeout) {
                if (process_message()) break;

                tries++;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }

        bool is_correct_id(uint32_t index, uint64_t id) {
            if (index > m_connections.size() - 1) return false;

            auto front = m_connections.begin();
            std::advance(front, index);
            return (*front)->id() == id;
        }
};

class test_client : public tcp_client<commands> {
    public:
        test_client() : tcp_client() {}

        bool server_said_echo = false;
        bool server_said_broadcast = false;
        std::string pong = "";

        void on_message(message_t& msg) override {
            switch (msg.head.command) {
                case commands::s2c_echo:
                    server_said_echo = true;
                    break;
                case commands::s2c_pong:
                    msg >> pong;
                    break;
                case commands::s2c_broadcast:
                    server_said_broadcast = true;
                    break;

                default:
                    break;
            }
        }

        void on_disconnect() override {

        }

        void wait_for_msg(const int timeout = 30) {
            int tries = 0;
            while (tries < timeout) {
                if (process_message()) break;

                tries++;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
};

void service_connect() {
    test_service server;
    server.start("127.0.0.1", 21205);

    test_client client;
    bool connected = client.connect("127.0.0.1", 21205);

    ASSERT(connected == true);
}

void service_client_hello() {
    test_service server;
    server.start("127.0.0.1", 21205);

    test_client client;
    client.connect("127.0.0.1", 21205);

    test_client::message_t msg(commands::c2s_hello);
    client.send(msg);

    server.wait_for_msg();
    ASSERT(server.client_said_hello == true);
}

void service_echo() {
    test_service server;
    server.start("127.0.0.1", 21205);

    test_client client;
    client.connect("127.0.0.1", 21205);

    test_client::message_t msg(commands::c2s_echo);
    client.send(msg);

    server.wait_for_msg();
    ASSERT(server.client_said_echo == true);

    client.wait_for_msg();
    ASSERT(client.server_said_echo == true);
}

void service_ping_pong() {
    test_service server;
    server.start("127.0.0.1", 21205);

    test_client client;
    client.connect("127.0.0.1", 21205);

    test_client::message_t msg(commands::c2s_ping);
    msg << std::string("PiNg");
    client.send(msg);

    server.wait_for_msg();
    ASSERT(server.ping == "PiNg");

    client.wait_for_msg();
    ASSERT(client.pong == "pOnG");
}

void service_broadcast() {
    test_service server;
    server.start("127.0.0.1", 21205);

    test_client client1;
    ASSERT(client1.connect("127.0.0.1", 21205) == true);

    test_client client2;
    ASSERT(client2.connect("127.0.0.1", 21205) == true);

    ASSERT(server.is_correct_id(0, 1));
    ASSERT(server.is_correct_id(1, 2));

    test_client::message_t msg(commands::c2s_broadcast);
    client1.send(msg);

    server.wait_for_msg();
    ASSERT(server.client_said_broadcast == true);

    client1.wait_for_msg();
    ASSERT(client1.server_said_broadcast == true);

    client2.wait_for_msg();
    ASSERT(client2.server_said_broadcast == true);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        service_connect();
        service_client_hello();
        service_echo();
        service_ping_pong();
        service_broadcast();
    }
    else {
        switch (std::stoi(argv[1])) {
            case 0: service_connect();      break;
            case 1: service_client_hello(); break;
            case 2: service_echo();         break;
            case 3: service_ping_pong();    break;
            case 4: service_broadcast();    break;
            default: break;
        }
    }

    return 0;
}