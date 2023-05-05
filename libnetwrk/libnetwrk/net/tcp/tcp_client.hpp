#ifndef LIBNETWRK_NET_TCP_TCP_CLIENT_HPP
#define LIBNETWRK_NET_TCP_TCP_CLIENT_HPP

#include <thread>
#include <exception>

#include "libnetwrk/net/definitions.hpp"
#include "libnetwrk/net/common/base_client.hpp"
#include "libnetwrk/net/tcp/tcp_connection.hpp"

namespace libnetwrk::net::tcp {
	template <typename command_type, 
		typename serializer = libnetwrk::net::common::binary_serializer,
		typename storage = libnetwrk::nothing>
	class tcp_client 
		: public libnetwrk::net::common::base_client<command_type, serializer, storage> 
	{
		public:
			typedef libnetwrk::net::common::base_client<command_type, serializer, storage> base_t;

			typedef base_t::message_t			message_t;
			typedef base_t::message_t_ptr		message_t_ptr;
			typedef base_t::owned_message_t		owned_message_t;
			typedef command_type				cmd_t;

		private:
			typedef tcp_connection<command_type, serializer, storage> tcp_connection_t;
			typedef std::shared_ptr<tcp_connection_t> tcp_connection_t_ptr;

		public:
			tcp_client(const std::string& name = "tcp client") : base_t(name) {}
		
			virtual ~tcp_client() {}

		protected:
			virtual void on_message(message_t& msg) override {}

			virtual void on_disconnect() override {}

		private:
			bool _connect(const char* host, const unsigned short port) override {
				try {
					// Create ASIO context
					this->m_context = std::make_shared<asio::io_context>(1);

					// Create ASIO endpoint
					asio::ip::tcp::endpoint ep(asio::ip::address::from_string(host), port);

					// Create ASIO socket
					asio::ip::tcp::socket socket(*(this->m_context), ep.protocol());

					// Connect
					socket.connect(ep);

					// Create connection object
					this->m_connection = 
						std::make_shared<tcp_connection_t>(*this, std::move(socket));

					// Start receiving messages
					this->m_connection->start();

					// Start ASIO context
					this->start_context();

					this->m_connected = true;

					LIBNETWRK_INFO("connected to %s:%d", host, port);
				}
				catch (const std::exception& e) {
					LIBNETWRK_ERROR("failed to connect | %s", e.what());
					this->teardown();
					return false;
				}
				catch (...) {
					LIBNETWRK_ERROR("failed to connect | fatal error");
					this->teardown();
					return false;
				}

				return true;
			}
	};
}

#endif