#pragma once

#include "libnetwrk/net/default_service_desc.hpp"
#include "libnetwrk/net/tcp/socket.hpp"
#include "libnetwrk/net/tcp/tcp_resolver.hpp"
#include "libnetwrk/net/core/client/client.hpp"
#include "libnetwrk/net/serialization/bin_serialize.hpp"

#include <exception>
#include <thread>

namespace libnetwrk::tcp {
    template<typename tn_desc = libnetwrk::default_service_desc>
    requires is_libnetwrk_service_desc<tn_desc>
    class tcp_client : public libnetwrk::client<tn_desc, libnetwrk::tcp::socket> {
    public:
        using base_t          = libnetwrk::client<tn_desc, libnetwrk::tcp::socket>;
        using connection_t    = base_t::connection_t;
        using command_t       = typename tn_desc::command_t;
        using message_t       = base_t::message_t;
        using owned_message_t = base_t::owned_message_t;

    public:
        tcp_client(const std::string& name = "TCP client") 
            : base_t(name) {}

        virtual ~tcp_client() {
            this->m_context.status = service_status::stopping;
            this->teardown();
        };

    private:
        // Native socket type for this client
        using native_socket_t = libnetwrk::tcp::socket::native_socket_t;

    private:
        bool connect_impl(const std::string& host, const uint16_t port) override final {
            try {
                // Create resolver
                tcp_resolver resolver(this->m_context.io_context);

                // Resolve hostname
                asio::ip::tcp::endpoint ep;
                if (!resolver.get_endpoint(host, port, ep))
                    throw libnetwrk_exception("Failed to resolve hostname.");

                // Create connection object
                this->m_comp_connection.create_connection();

                // Connect
                this->m_comp_connection.establish_connection(ep);

                // Start read/write
                this->m_comp_message.start_connection_read_and_write(this->m_comp_connection.connection);

                // Start context
                this->m_context.start_io_context();

                LIBNETWRK_INFO(this->m_context.name, "Connected to {}:{}.", host, port);
            }
            catch (const std::exception& e) {
                (void)e;

                LIBNETWRK_ERROR(this->m_context.name, "Failed to connect. | {}", e.what());
                this->teardown();
                return false;
            }
            catch (...) {
                LIBNETWRK_ERROR(this->m_context.name, "Failed to connect. | Critical fail.");
                this->teardown();
                return false;
            }

            return true;
        }
    };
}
