#include "libnetwrk.hpp"
#include "commands.hpp"

#include <iostream>

using namespace libnetwrk::tcp;
using namespace libnetwrk;

class tcp_echo_client : public tcp_client<commands> {
public:
    tcp_echo_client() : tcp_client() {}

    void ev_message(owned_message_t& msg) override {
        switch (msg.msg.head.command) {
            case commands::s2c_echo:
            {
                std::string text;
                msg.msg >> text;
                LIBNETWRK_INFO(this->name, "{}", text);
                break;
            }
            default: break;
        }
    }
};

int main(int argc, char* argv[]) {
    dvsku::log::init(dvsku::log_level::verbose, true);

    tcp_echo_client client;
    client.connect("127.0.0.1", 21205);
    client.process_messages_async();

    while (true) {
        std::string input;
        std::getline(std::cin, input);

        if (input == "q") break;

        tcp_echo_client::message_t msg(commands::c2s_echo);
        msg << input;
        client.send(msg);
    }

    client.disconnect();

    return 0;
}