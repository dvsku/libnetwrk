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
			typedef libnetwrk::net::owned_message<command_type, serializer, storage> owned_message_t;

		protected:
			asio::ip::tcp::socket m_socket;

		public:
			tcp_connection(libnetwrk::net::common::connection_owner owner,
				asio::ip::tcp::socket socket, context_ptr context,
				libnetwrk::net::common::tsdeque<owned_message_t>& queue)
				: libnetwrk::net::common::base_connection<command_type, serializer, storage>(owner, context, queue),
				m_socket(std::move(socket)) {}

			const std::string remote_address() override {
				return m_socket.remote_endpoint().address().to_string();
			}

			const unsigned short remote_port() override {
				return m_socket.remote_endpoint().port();
			}

			void stop() override {
				m_socket.close();
			}

			bool is_alive() override {
				return m_socket.is_open();
			}

		protected:
			void read_verification_message() override {
				if (this->m_owner == libnetwrk::net::common::connection_owner::server) {
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
				asio::async_read(m_socket, asio::buffer(&this->m_temp_message.m_head, 
					sizeof(libnetwrk::net::message_head<command_type>)),
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
				if (this->m_owner == libnetwrk::net::common::connection_owner::server) {
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
				asio::async_write(m_socket, asio::buffer(&this->m_outgoing_messages.front().m_head, 
					sizeof(libnetwrk::net::message_head<command_type>)),
					std::bind(&tcp_connection::write_message_head_callback, 
						this, std::placeholders::_1, std::placeholders::_2));
			}

			void write_message_body() override {
				asio::async_write(m_socket, asio::buffer(this->m_outgoing_messages.front().m_data.data(),
					this->m_outgoing_messages.front().m_data.size()),
					std::bind(&tcp_connection::write_message_body_callback, 
						this, std::placeholders::_1, std::placeholders::_2));
			}

			void on_disconnect() override {
				libnetwrk::net::common::base_connection<command_type, serializer, storage>::on_disconnect();
			}

			void on_error(std::error_code ec) override {
				libnetwrk::net::common::base_connection<command_type, serializer, storage>::on_error(ec);
			}
	};
}

#endif