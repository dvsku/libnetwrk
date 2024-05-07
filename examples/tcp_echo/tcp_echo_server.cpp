#include "example_logging.hpp"
#include "commands.hpp"

#include <iostream>

using namespace libnetwrk::tcp;
using namespace libnetwrk;

class tcp_echo_server : public tcp_server<commands> {
public:
    tcp_echo_server() : tcp_server() {}

    void ev_message(owned_message_t& msg) override {
        message_t response;
        switch (msg.msg.head.command) {
            case commands::c2s_echo: {
                std::string text;
                msg.msg >> text;

                LIBNETWRK_INFO(this->name, "{}:{}\t{}",
                    msg.sender->remote_address().c_str(), msg.sender->remote_port(), text);

                response.head.command = commands::s2c_echo;
                response << text;

                msg.sender->send(response);
                break;
            }
            default: break;
        }
    }
};

int main(int argc, char* argv[]) {
    dvsku::dv_util_log::settings log_settings;
    log_settings.log_to_file = false;
    log_settings.level       = dvsku::dv_util_log::level::verbose;

    dvsku::dv_util_log::init(log_settings);
    dvsku::dv_util_log::create_source("console", &std::cout);

    tcp_echo_server server;
    server.start("127.0.0.1", 21205);
    server.process_messages();

    server.stop();

    return 0;
}