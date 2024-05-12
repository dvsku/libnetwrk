#include "service_desc.hpp"

#include <iostream>

using namespace libnetwrk::tcp;
using namespace libnetwrk;

class tcp_echo_service : public tcp_service<service_desc> {
public:
    tcp_echo_service() : tcp_service() {}

    void ev_message(owned_message_t& msg) override {
        message_t response;
        switch (msg.msg.command()) {
            case commands::c2s_echo: {
                std::string text;
                msg.msg >> text;

                LIBNETWRK_INFO(this->name, "{}:{}\t{}",
                    msg.sender->get_ip().c_str(), msg.sender->get_port(), text);

                response.set_command(commands::s2c_echo);
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

    tcp_echo_service service;
    service.start("127.0.0.1", 21205);
    service.process_messages();

    service.stop();

    return 0;
}