#pragma once

#include "libnetwrk/net/default_service_desc.hpp"
#include "libnetwrk/net/tcp/socket.hpp"
#include "libnetwrk/net/core/base_client.hpp"
#include "libnetwrk/net/core/serialization/bin_serialize.hpp"

#include <exception>
#include <thread>

namespace libnetwrk::tcp {
    template<typename Desc = libnetwrk::default_service_desc>
    requires is_libnetwrk_service_desc<Desc>
    class tcp_client : public libnetwrk::base_client<Desc, libnetwrk::tcp::socket> {
    public:
        using base_t            = libnetwrk::base_client<Desc, libnetwrk::tcp::socket>;
        using message_t         = base_t::message_t;
        using owned_message_t   = base_t::owned_message_t;
        using connection_t      = base_t::connection_t;
        using base_connection_t = connection_t::base_connection_t;
        using native_socket_t   = libnetwrk::tcp::socket::native_socket_t;
        using command_t         = typename Desc::command_t;


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
                native_socket_t socket(*(this->asio_context), ep.protocol());

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
                (void)e;

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
