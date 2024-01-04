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
        using base_connection_t = base_t::connection_t;

        using guard_t    = std::lock_guard<std::mutex>;
        using acceptor_t = asio::ip::tcp::acceptor;
        using socket_t   = asio::ip::tcp::socket;

    public:
        tcp_server(const std::string& name = "tcp server") 
            : base_t(name) {};

        virtual ~tcp_server() {
            stop();
        }

        /// <summary>
        /// Stop server
        /// </summary>
        void stop() override {
            if (m_acceptor)
                if (m_acceptor->is_open())
                    m_acceptor->close();

            base_t::stop();
        }

    protected:
        std::unique_ptr<acceptor_t> m_acceptor;

    protected:
        virtual void on_message(owned_message_t& msg) override {}

        virtual bool on_before_client_connect(std::shared_ptr<base_connection_t> client) {
            return true;
        }

        virtual void on_client_connect(std::shared_ptr<connection_t> client) {}

        virtual void on_client_disconnect(std::shared_ptr<base_connection_t> client) override {
            base_t::on_client_disconnect(client);
        }

        bool _start(const char* host, const unsigned short port) override {
            try {
                // Create ASIO context
                this->asio_context = std::make_unique<asio::io_context>(1);

                // Create ASIO acceptor
                m_acceptor = std::make_unique<acceptor_t>
                    (*(this->asio_context), asio::ip::tcp::endpoint(asio::ip::address::from_string(host), port));

                // Start listening for and accepting connections
                _accept();

                // Start ASIO context
                this->start_context();

                this->m_running = true;

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

        void _accept() override {
            m_acceptor->async_accept(
                [this](std::error_code ec, socket_t socket) {
                    if (!ec) {
                        LIBNETWRK_VERBOSE(this->name, "attempted connection from {}:{}",
                            socket.remote_endpoint().address().to_string(),
                            socket.remote_endpoint().port());

                        auto new_connection =  std::make_shared<connection_t>(*this, std::move(socket));

                        if (on_before_client_connect(new_connection)) {
                            guard_t guard(this->m_connections_mutex);

                            this->m_connections.push_back(new_connection);
                            this->m_connections.back()->id() = ++this->m_ids;
                            this->m_connections.back()->start();
                            on_client_connect(new_connection);

                            LIBNETWRK_INFO(this->name, "connection success from {}:{}",
                                this->m_connections.back()->remote_address(),
                                this->m_connections.back()->remote_port());
                        }
                        else {
                            LIBNETWRK_WARNING(this->name, "connection denied");
                        }
                    }
                    else if (ec == asio::error::operation_aborted) {
                        LIBNETWRK_INFO(this->name, "listening stopped");
                        return;
                    }
                    else {
                        LIBNETWRK_ERROR(this->name, "failed to accept connection | {}", ec.message());
                    }

                    _accept();
                }
            );
        };
    };
}
