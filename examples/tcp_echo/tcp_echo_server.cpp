#include "libnetwrk.hpp"
#include "commands.hpp"

using namespace libnetwrk::net::tcp;
using namespace libnetwrk::net;

class tcp_echo_server : public tcp_server<commands> {
    public:
        tcp_echo_server() : tcp_server() {}

        void on_message(owned_message_t& msg) override {
            message<commands> response;
            switch (msg.m_msg.m_head.m_command) {
                case commands::c2s_echo: {
                    std::string text;
                    msg.m_msg >> text;

                    LIBNETWRK_INFO(this->name(), "{}:{}\t{}",
                        msg.m_client->remote_address().c_str(), msg.m_client->remote_port(), text);

                    response.m_head.m_command = commands::s2c_echo;
                    response << text;

                    msg.m_client->send(response);
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