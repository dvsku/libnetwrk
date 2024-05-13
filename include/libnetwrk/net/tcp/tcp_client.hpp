#pragma once

#include "libnetwrk/net/default_service_desc.hpp"
#include "libnetwrk/net/tcp/socket.hpp"
#include "libnetwrk/net/tcp/tcp_resolver.hpp"
#include "libnetwrk/net/core/base_client.hpp"
#include "libnetwrk/net/core/serialization/bin_serialize.hpp"

#include <exception>
#include <thread>

namespace libnetwrk::tcp {
    template<typename Desc = libnetwrk::default_service_desc>
    requires is_libnetwrk_service_desc<Desc>
    class tcp_client : public libnetwrk::base_client<Desc, libnetwrk::tcp::socket> {
    public:
        // This client type
        using client_t = tcp_client<Desc>;

        // Base client type
        using base_client_t = libnetwrk::base_client<Desc, libnetwrk::tcp::socket>;

        // Connection type for this client
        using connection_t = base_client_t::connection_t;

        // Message type
        using message_t = base_client_t::message_t;

        // Owned message type for this client
        using owned_message_t = base_client_t::owned_message_t;

        // Command type for this client
        using command_t = typename Desc::command_t;

    public:
        tcp_client(const std::string& name = "TCP client") 
            : base_client_t(name) {}

        virtual ~tcp_client() {
            if (this->m_status == service_status::stopped || this->m_status == service_status::stopping)
                return;

            this->m_status = service_status::stopping;
            teardown();
        };

    public:
        /*
            Disconnect the client and clean up.
        */
        void disconnect() override final {
            if (this->m_status != service_status::started)
                return;

            this->m_status = service_status::stopping;

            teardown();
            base_client_t::teardown();
            ev_disconnected();

            this->m_status = service_status::stopped;
        }

    protected:
        /*
            Called when successfully connected.
        */
        virtual void ev_connected() override {};

        /*
            Called when disconnected.
        */
        virtual void ev_disconnected() override {};

        /*
            Called when processing messages.
        */
        virtual void ev_message(owned_message_t& msg) override {};

    protected:
        /*
            Pre process message data before writing.
        */
        virtual void pre_process_message(message_t::buffer_t& buffer) override {}

        /*
            Post process message data after reading.
        */
        virtual void post_process_message(message_t::buffer_t& buffer) override {}

    private:
        // Native socket type for this client
        using native_socket_t = libnetwrk::tcp::socket::native_socket_t;

    private:
        void teardown() {};

        bool impl_connect(const char* host, const unsigned short port) override final {
            try {
                // Create ASIO context
                this->io_context = std::make_unique<asio::io_context>(1);

                // Create resolver
                tcp_resolver resolver(*this);

                // Resolve hostname
                asio::ip::tcp::endpoint ep;
                if (!resolver.get_endpoint(host, port, ep))
                    throw libnetwrk_exception("Failed to resolve hostname.");

                // Create ASIO socket
                native_socket_t socket(*(this->io_context), ep.protocol());

                // Connect
                socket.connect(ep);

                // Create connection object
                this->m_connection =
                    std::make_shared<connection_t>(*this, std::move(socket));

                // Start receiving messages
                this->m_connection->start();

                // Start ASIO context
                this->start_context();

                LIBNETWRK_INFO(this->name, "Connected to {}:{}.", host, port);
            }
            catch (const std::exception& e) {
                (void)e;

                LIBNETWRK_ERROR(this->name, "Failed to connect. | {}", e.what());
                this->teardown();
                return false;
            }
            catch (...) {
                LIBNETWRK_ERROR(this->name, "Failed to connect. | Critical fail.");
                this->teardown();
                return false;
            }

            return true;
        }
    };
}
