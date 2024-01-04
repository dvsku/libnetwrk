#pragma once

#include "libnetwrk/net/core/base_connection.hpp"

namespace libnetwrk::tcp {
    template<typename Tcommand, typename Tserialize, typename Tstorage>
    class tcp_connection : public libnetwrk::base_connection<Tcommand, Tserialize, Tstorage> {
    public:
        using base_t          = libnetwrk::base_connection<Tcommand, Tserialize, Tstorage>;
        using base_context_t  = base_t::base_context_t;
        using owned_message_t = base_t::owned_message_t;
        using socket_t        = asio::ip::tcp::socket;

    public:
        tcp_connection(base_context_t& context, socket_t socket)
            : base_t(context), m_socket(std::move(socket)) {}

        /// <summary>
        /// Get IPv4 address
        /// </summary>
        const std::string remote_address() override {
            return m_socket.remote_endpoint().address().to_string();
        }

        /// <summary>
        /// Get port
        /// </summary>
        const unsigned short remote_port() override {
            return m_socket.remote_endpoint().port();
        }

        /// <summary>
        /// Close connection
        /// </summary>
        void stop() override {
            m_socket.close();
        }

        /// <summary>
        /// Get connection status
        /// </summary>
        /// <returns>true if open, false if closed</returns>
        bool is_alive() override {
            return m_socket.is_open();
        }

    protected:
        socket_t m_socket;

    protected:
        void read_verification_message() override {
            if (this->m_context.owner == context_owner::server) {
                asio::async_read(m_socket, asio::buffer(&this->m_verification_code, sizeof(uint32_t)),
                    std::bind(&tcp_connection::read_verification_message_callback,
                        this, std::placeholders::_1, std::placeholders::_2));
            }
            else {
                std::error_code ec;
                size_t written = asio::read(m_socket,
                    asio::buffer(&this->m_verification_code, sizeof(uint32_t)), ec);
                this->read_verification_message_callback(ec, written);
            }
        }

        void read_message_head() override {
            asio::async_read(m_socket,
                asio::buffer(this->m_recv_message.data_head.data(),
                    this->m_recv_message.head.size()),
                std::bind(&tcp_connection::read_message_head_callback,
                    this, std::placeholders::_1, std::placeholders::_2));
        }

        void read_message_body() override {
            asio::async_read(m_socket, asio::buffer(this->m_recv_message.data.data(),
                this->m_recv_message.head.data_size),
                std::bind(&tcp_connection::read_message_body_callback,
                    this, std::placeholders::_1, std::placeholders::_2));
        }

        void write_verification_message() override {
            if (this->m_context.owner == context_owner::server) {
                asio::async_write(m_socket, asio::buffer(&this->m_verification_code, sizeof(uint32_t)),
                    std::bind(&tcp_connection::write_verification_message_callback,
                        this, std::placeholders::_1, std::placeholders::_2));
            }
            else {
                std::error_code ec;
                size_t written = asio::write(m_socket,
                    asio::buffer(&this->m_verification_code, sizeof(uint32_t)), ec);
                this->write_verification_message_callback(ec, written);
            }
        }

        void write_message_head() override {
            if (this->m_outgoing_messages.front()->data_head.empty()) {
                this->m_outgoing_messages.front()->head.send_timestamp =
                    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

                this->m_outgoing_messages.front()->head.serialize(this->m_outgoing_messages.front()->data_head);
            }

            asio::async_write(m_socket,
                asio::buffer(this->m_outgoing_messages.front()->data_head.data(),
                    this->m_outgoing_messages.front()->data_head.size()),
                std::bind(&tcp_connection::write_message_head_callback,
                    this, std::placeholders::_1, std::placeholders::_2));
        }

        void write_message_body() override {
            asio::async_write(m_socket,
                asio::buffer(this->m_outgoing_messages.front()->data.data(),
                    this->m_outgoing_messages.front()->data.size()),
                std::bind(&tcp_connection::write_message_body_callback,
                    this, std::placeholders::_1, std::placeholders::_2));
        }
    };
}
