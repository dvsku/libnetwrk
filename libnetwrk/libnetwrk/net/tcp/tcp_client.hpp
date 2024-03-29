#pragma once

#include "libnetwrk/net/core/base_client.hpp"
#include "libnetwrk/net/core/serialization/bin_serialize.hpp"
#include "libnetwrk/net/tcp/tcp_connection.hpp"

#include <exception>
#include <thread>

namespace libnetwrk::tcp {
    template<typename Command, typename Serialize = libnetwrk::bin_serialize, typename Storage = libnetwrk::nothing>
    requires is_enum<Command>
    class tcp_client : public libnetwrk::base_client<Command, Serialize, Storage> {
    public:
        using base_t          = libnetwrk::base_client<Command, Serialize, Storage>;
        using connection_t    = tcp_connection<Command, Serialize, Storage>;
        using message_t       = base_t::message_t;
        using owned_message_t = base_t::owned_message_t;
        using command_t       = Command;

    public:
        tcp_client(const std::string& name = "tcp client") 
            : base_t(name) {}

        virtual ~tcp_client() = default;

    protected:
        // Called when successfully connected
        virtual void ev_connected() override {};

        // Called when disconnected
        virtual void ev_disconnected() override {};

        // Called when processing messages
        virtual void ev_message(owned_message_t& msg) override {};

    private:
        bool impl_connect(const char* host, const unsigned short port) override final {
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
