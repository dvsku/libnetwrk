#pragma once

#include "libnetwrk/net/core/base_server.hpp"
#include "libnetwrk/net/core/serialization/bin_serialize.hpp"
#include "libnetwrk/net/tcp/tcp_connection.hpp"

#include <exception>
#include <thread>

namespace libnetwrk::tcp {
    template<typename Command, typename Serialize = libnetwrk::bin_serialize, typename Storage = libnetwrk::nothing>
    class tcp_server : public libnetwrk::base_server<Command, Serialize, Storage> {
    public:
        using tcp_server_t      = tcp_server<Command, Serialize, Storage>;
        using base_t            = libnetwrk::base_server<Command, Serialize, Storage>;
        using message_t         = base_t::message_t;
        using owned_message_t   = base_t::owned_message_t;
        using command_t         = Command;
        using connection_t      = tcp_connection<Command, Serialize, Storage>;
        using base_connection_t = base_t::base_connection_t;

        using guard_t    = std::lock_guard<std::mutex>;
        using acceptor_t = asio::ip::tcp::acceptor;
        using socket_t   = asio::ip::tcp::socket;

    public:
        tcp_server(const std::string& name = "tcp server") 
            : base_t(name) {};

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
            base_t::teardown();

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
        virtual bool ev_before_client_connected(std::shared_ptr<base_connection_t> client) override { return true; };

        // Called when a client has connected
        virtual void ev_client_connected(std::shared_ptr<base_connection_t> client) override {};

        // Called when a client has disconnected
        virtual void ev_client_disconnected(std::shared_ptr<base_connection_t> client) override {};

    protected:
        void teardown() {
            if (m_acceptor && m_acceptor->is_open())
                m_acceptor->close();
        };

    private:
        bool impl_start(const char* host, const unsigned short port) override final {
            try {
                // Create ASIO context
                this->asio_context = std::make_unique<asio::io_context>(1);

                // Create ASIO acceptor
                m_acceptor = std::make_unique<acceptor_t>
                    (*(this->asio_context), asio::ip::tcp::endpoint(asio::ip::address::from_string(host), port));

                // Start listening for and accepting connections
                impl_accept();

                // Start ASIO context
                this->start_context();

                LIBNETWRK_INFO(this->name, "listening for connections on {}:{}", host, port);
            }
            catch (const std::exception& e) {
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
                [this](std::error_code ec, socket_t socket) {
                    if (!ec) {
                        LIBNETWRK_VERBOSE(this->name, "attempted connection from {}:{}",
                            socket.remote_endpoint().address().to_string(),
                            socket.remote_endpoint().port());

                        auto new_connection =  std::make_shared<connection_t>(*this, std::move(socket));

                        if (ev_before_client_connected(new_connection)) {
                            guard_t guard(this->m_connections_mutex);

                            this->m_connections.push_back(new_connection);
                            this->m_connections.back()->id() = ++this->m_ids;
                            this->m_connections.back()->start();
                            ev_client_connected(new_connection);

                            LIBNETWRK_INFO(this->name, "connection success from {}:{}",
                                this->m_connections.back()->remote_address(),
                                this->m_connections.back()->remote_port());
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
