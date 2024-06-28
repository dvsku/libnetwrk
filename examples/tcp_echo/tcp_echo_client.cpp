#include "service_desc.hpp"

#include <iostream>

using namespace libnetwrk::tcp;
using namespace libnetwrk;

class tcp_echo_client : public tcp_client<service_desc> {
public:
    tcp_echo_client() : tcp_client<service_desc>() {
        set_message_callback([this](auto command, auto message) {
            ev_message(command, message);
        });
    }

protected:
    void ev_message(command_t command, owned_message_t* msg) {
        switch (command) {
            case commands::s2c_echo:
            {
                std::string text;
                msg->message >> text;
                DV_LOG_INFO(get_name(), "{}", text);
                break;
            }
            default: break;
        }
    }
};

int main(int argc, char* argv[]) {
    libutil::log::settings log_settings;
    log_settings.log_to_file = false;
    log_settings.level       = libutil::log::level::debug;

    libutil::log::init(log_settings);

    tcp_echo_client client;
    client.connect("127.0.0.1", 21205);
    client.process_messages_async();

    while (client.is_connected()) {
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