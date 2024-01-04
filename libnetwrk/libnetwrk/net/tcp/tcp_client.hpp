#pragma once

#include "libnetwrk/net/core/base_client.hpp"
#include "libnetwrk/net/core/serialization/bin_serialize.hpp"
#include "libnetwrk/net/tcp/tcp_connection.hpp"

#include <exception>
#include <thread>

namespace libnetwrk::tcp {
    template <typename Tcommand, typename Tserialize = libnetwrk::bin_serialize, typename Tstorage = libnetwrk::nothing>
    requires is_enum<Tcommand>
    class tcp_client : public libnetwrk::base_client<Tcommand, Tserialize, Tstorage> {
    public:
        using base_t          = libnetwrk::base_client<Tcommand, Tserialize, Tstorage>;
        using connection_t    = tcp_connection<Tcommand, Tserialize, Tstorage>;
        using message_t       = base_t::message_t;
        using owned_message_t = base_t::owned_message_t;
        using command_t       = Tcommand;

    public:
        tcp_client(const std::string& name = "tcp client") 
            : base_t(name) {}

        virtual ~tcp_client() {}

    protected:
        virtual void on_message(message_t& msg) override {}

        virtual void on_connect()    override {}
        virtual void on_disconnect() override {}

    private:
        bool _connect(const char* host, const unsigned short port) override {
            try {
                // Create ASIO context
                this->asio_context = std::make_unique<asio::io_context>(1);

                // Create ASIO endpoint
                asio::ip::tcp::endpoint ep(asio::ip::address::from_string(host), port);

                // Create ASIO socket
                asio::ip::tcp::socket socket(*(this->asio_context), ep.protocol());

                // Connect
                socket.connect(ep);

                // Create connection object
                this->m_connection =
                    std::make_shared<connection_t>(*this, std::move(socket));

                // Start receiving messages
                this->m_connection->start();

                // Start ASIO context
                this->start_context();

                this->m_connected = true;

                on_connect();

                LIBNETWRK_INFO(this->name, "connected to {}:{}", host, port);
            }
            catch (const std::exception& e) {
                LIBNETWRK_ERROR(this->name, "failed to connect | {}", e.what());
                this->teardown();
                return false;
            }
            catch (...) {
                LIBNETWRK_ERROR(this->name, "failed to connect | fatal error");
                this->teardown();
                return false;
            }

            return true;
        }
    };
}
