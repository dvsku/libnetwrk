#include "libnetwrk.hpp"
#include "commands.hpp"

using namespace libnetwrk::tcp;
using namespace libnetwrk;

class tcp_echo_server : public tcp_server<commands> {
public:
    tcp_echo_server() : tcp_server() {}

    void on_message(owned_message_t& msg) override {
        message_t response;
        switch (msg.message.head.command) {
            case commands::c2s_echo: {
                std::string text;
                msg.message >> text;

                LIBNETWRK_INFO(this->name, "{}:{}\t{}",
                    msg.client->remote_address().c_str(), msg.client->remote_port(), text);

                response.head.command = commands::s2c_echo;
                response << text;

                msg.client->send(response);
                break;
            }
            default: break;
        }
    }
};

int main(int argc, char* argv[]) {
    tcp_echo_server server;
    server.start("127.0.0.1", 21205);
    server.process_messages();

    server.stop();

    return 0;
}