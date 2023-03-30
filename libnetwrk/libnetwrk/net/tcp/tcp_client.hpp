#ifndef LIBNETWRK_NET_TCP_TCP_CLIENT_HPP
#define LIBNETWRK_NET_TCP_TCP_CLIENT_HPP

#include <thread>
#include <exception>
#include <type_traits>

#include "libnetwrk/net/definitions.hpp"
#include "libnetwrk/net/message.hpp"
#include "libnetwrk/net/macros.hpp"
#include "libnetwrk/net/common/containers/tsdeque.hpp"
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
			typedef libnetwrk::net::message<command_type, serializer> message_t;
			typedef std::shared_ptr<message_t> message_t_ptr;
			typedef libnetwrk::net::owned_message<command_type, serializer, storage> owned_message_t;
			typedef command_type cmd_t;

		private:
			typedef tcp_connection<command_type, serializer, storage> tcp_connection_t;
			typedef std::shared_ptr<tcp_connection_t> tcp_connection_t_ptr;
			typedef libnetwrk::net::common::base_client<command_type, serializer, storage> base;

		public:
			tcp_client(const std::string& name = "tcp client") 
				: libnetwrk::net::common::base_client<command_type, serializer, storage>(name) {}
		
			virtual ~tcp_client() {}

		protected:
			virtual void on_message(message_t& msg) override {}

			virtual void on_disconnect() override {}

		private:
			bool _connect(const char* host, const unsigned short port) override {
				try {
					// Create ASIO context
					base::m_context = std::make_shared<asio::io_context>(1);

					// Create ASIO endpoint
					asio::ip::tcp::endpoint ep(asio::ip::address::from_string(host), port);

					// Create ASIO socket
					asio::ip::tcp::socket socket(*(base::m_context), ep.protocol());

					// Connect
					socket.connect(ep);

					// Create connection object
					base::m_connection =
						std::make_shared<tcp_connection_t>(libnetwrk::net::common::connection_owner::client,
							std::move(socket), base::m_context, base::m_incoming_messages);

					// Start receiving messages
					base::m_connection->start();

					// Start ASIO context
					base::m_context_thread = std::thread([this] { base::m_context->run(); });

					base::m_connected = true;

					LIBNETWRK_INFO("connected to %s:%d", host, port);
				}
				catch (const std::exception& e) {
					LIBNETWRK_ERROR("failed to connect | %s", e.what());
					base::stop();
					return false;
				}
				catch (...) {
					LIBNETWRK_ERROR("failed to connect | fatal error");
					base::stop();
					return false;
				}

				return true;
			}
	};
}

#endif