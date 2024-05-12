#pragma once

#include "libnetwrk/net/default_service_desc.hpp"
#include "libnetwrk/net/tcp/socket.hpp"
#include "libnetwrk/net/tcp/tcp_resolver.hpp"
#include "libnetwrk/net/core/base_server.hpp"
#include "libnetwrk/net/core/serialization/bin_serialize.hpp"

#include <exception>
#include <thread>

namespace libnetwrk::tcp {
    template<typename Desc = libnetwrk::default_service_desc>
    requires is_libnetwrk_service_desc<Desc>
    class tcp_server : public libnetwrk::base_server<Desc, libnetwrk::tcp::socket> {
    public:
        // This service type
        using service_t = tcp_server<Desc>;

        // Base service type
        using base_service_t = libnetwrk::base_server<Desc, libnetwrk::tcp::socket>;

        // Connection type for this service
        using connection_t = base_service_t::connection_t;

        // Message type
        using message_t = base_service_t::message_t;

        // Owned message type for this service
        using owned_message_t = base_service_t::owned_message_t;

        // Command type for this service
        using command_t = typename Desc::command_t;

    private:
        // Client acceptor type
        using acceptor_t = asio::ip::tcp::acceptor;

    public:
        tcp_server(const std::string& name = "tcp service") 
            : base_service_t(name) {};

        virtual ~tcp_server() {
            if (this->m_status == service_status::stopped || this->m_status == service_status::stopping)
                return;

            this->m_status = service_status::stopping;
            teardown();
        }

        /// <summary>
        /// Stop server
        /// </summary>
        void stop() override final {
            if (this->m_status != service_status::started)
                return;

            this->m_status = service_status::stopping;

            teardown();
            base_service_t::teardown();

            this->m_status = service_status::stopped;

            ev_service_stopped();
        }

    protected:
        std::unique_ptr<acceptor_t> m_acceptor;

    protected:
        // Called when the service was successfuly started
        virtual void ev_service_started() override {};

        // Called when service stopped
        virtual void ev_service_stopped() override {};

        // Called when processing messages
        virtual void ev_message(owned_message_t& msg) override {};

        // Called before client is fully accepted
        // Allows performing checks on client before accepting (blacklist, whitelist)
        virtual bool ev_before_client_connected(std::shared_ptr<connection_t> client) override { return true; };

        // Called when a client has connected
        virtual void ev_client_connected(std::shared_ptr<connection_t> client) override {};

        // Called when a client has disconnected
        virtual void ev_client_disconnected(std::shared_ptr<connection_t> client) override {};

    private:
        // Native socket type for this service
        using native_socket_t = libnetwrk::tcp::socket::native_socket_t;

    private:
        void teardown() {
            if (m_acceptor && m_acceptor->is_open()) {
                m_acceptor->close();
                m_acceptor.reset();
            }
        };

        bool impl_start(const char* host, const unsigned short port) override final {
            try {
                // Create ASIO context
                this->io_context = std::make_unique<asio::io_context>(1);

                // Create resolver
                tcp_resolver resolver(*this);

                // Resolve hostname
                asio::ip::tcp::endpoint ep;
                if (!resolver.get_endpoint(host, port, ep))
                    throw libnetwrk_exception("failed to resolve hostname");

                // Create ASIO acceptor
                m_acceptor = std::make_unique<acceptor_t>(*(this->io_context), ep);

                // Start listening for and accepting connections
                impl_accept();

                // Start ASIO context
                this->start_context();

                LIBNETWRK_INFO(this->name, "listening for connections on {}:{}", host, port);
            }
            catch (const std::exception& e) {
                (void)e;

                LIBNETWRK_ERROR(this->name, "failed to start listening | {}", e.what());
                stop();
                return false;
            }
            catch (...) {
                LIBNETWRK_ERROR(this->name, "failed to start listening | fatal error");
                stop();
                return false;
            }

            return true;
        }

        void impl_accept() override final {
            m_acceptor->async_accept(
                [this](std::error_code ec, native_socket_t socket) {
                    if (!ec) {
                        LIBNETWRK_VERBOSE(this->name, "attempted connection from {}:{}",
                            socket.remote_endpoint().address().to_string(),
                            socket.remote_endpoint().port());

                        auto new_connection = std::make_shared<connection_t>(*this, std::move(socket));

                        if (ev_before_client_connected(new_connection)) {
                            std::lock_guard<std::mutex> guard(this->m_connections_mutex);

                            this->m_connections.push_back(new_connection);
                            this->m_connections.back()->id() = ++this->m_ids;
                            this->m_connections.back()->start();
                            ev_client_connected(new_connection);

                            LIBNETWRK_INFO(this->name, "connection success from {}:{}",
                                this->m_connections.back()->get_ip(),
                                this->m_connections.back()->get_port());
                        }
                        else {
                            LIBNETWRK_WARNING(this->name, "connection denied");
                        }
                    }
                    else if (ec == asio::error::operation_aborted) {
                        // Cancelled listening
                        return;
                    }
                    else {
                        // Should probably stop the server here
                        LIBNETWRK_ERROR(this->name, "failed to accept connection | {}", ec.message());
                    }

                    impl_accept();
                }
            );
        };
    };
}
