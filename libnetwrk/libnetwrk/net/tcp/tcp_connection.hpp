#ifndef LIBNETWRK_NET_TCP_TCP_CONNECTION_HPP
#define LIBNETWRK_NET_TCP_TCP_CONNECTION_HPP

#include "libnetwrk/net/common/base_connection.hpp"

namespace libnetwrk::net::tcp {
    template <typename command_type,
        typename serializer,
        typename storage = libnetwrk::nothing>
    class tcp_connection 
        : public libnetwrk::net::common::base_connection<command_type, serializer, storage>
    {
        public:
            typedef libnetwrk::net::common::base_connection<command_type, serializer, storage> base_t;

            typedef base_t::owned_message_t        owned_message_t;
            typedef base_t::base_context_t        base_context_t;

        protected:
            asio::ip::tcp::socket m_socket;

        public:
            tcp_connection(base_context_t& parent_context, asio::ip::tcp::socket socket)
                : base_t(parent_context), m_socket(std::move(socket)) {}

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
            void read_verification_message() override {
                if (this->m_parent_context.m_owner == connection_owner::server) {
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
                    asio::buffer(this->m_temp_message.m_head_data.data(),
                        this->m_temp_message.m_head_data.size()),
                    std::bind(&tcp_connection::read_message_head_callback, 
                        this, std::placeholders::_1, std::placeholders::_2));
            }

            void read_message_body() override {
                asio::async_read(m_socket, asio::buffer(this->m_temp_message.m_data.data(), 
                    this->m_temp_message.m_head.m_data_len),
                    std::bind(&tcp_connection::read_message_body_callback, 
                        this, std::placeholders::_1, std::placeholders::_2));
            }

            void write_verification_message() override {
                if (this->m_parent_context.m_owner == connection_owner::server) {
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
                this->m_outgoing_messages.front()->m_head.m_send_timestamp =
                    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

                this->m_outgoing_messages.front()->m_head_data =
                    this->m_outgoing_messages.front()->m_head.serialize();
                
                asio::async_write(m_socket, 
                    asio::buffer(this->m_outgoing_messages.front()->m_head_data.data(),
                        this->m_outgoing_messages.front()->m_head_data.size()),
                    std::bind(&tcp_connection::write_message_head_callback, 
                        this, std::placeholders::_1, std::placeholders::_2));
            }

            void write_message_body() override {
                asio::async_write(m_socket, 
                    asio::buffer(this->m_outgoing_messages.front()->m_data.data(),
                        this->m_outgoing_messages.front()->m_data.size()),
                    std::bind(&tcp_connection::write_message_body_callback, 
                        this, std::placeholders::_1, std::placeholders::_2));
            }
    };
}

#endif