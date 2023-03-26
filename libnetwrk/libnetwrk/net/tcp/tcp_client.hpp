#ifndef LIBNETWRK_NET_TCP_TCP_CLIENT_HPP
#define LIBNETWRK_NET_TCP_TCP_CLIENT_HPP

#include <thread>
#include <exception>
#include <type_traits>

#include "libnetwrk/net/message.hpp"
#include "libnetwrk/net/common/containers/tsdeque.hpp"
#include "libnetwrk/net/tcp/tcp_connection.hpp"
#include "libnetwrk/net/definitions.hpp"
#include "libnetwrk/net/macros.hpp"

namespace libnetwrk::net::tcp {
	template <typename command_type, 
		typename serializer = libnetwrk::net::common::binary_serializer,
		typename storage = libnetwrk::nothing>
	class tcp_client {
		public:
			typedef libnetwrk::net::message<command_type, serializer> message_t;
			typedef std::shared_ptr<message_t> message_t_ptr;
			typedef libnetwrk::net::owned_message<command_type, serializer, storage> owned_message_t;

			typedef tcp_connection<command_type, serializer, storage> tcp_connection_t;
			typedef std::shared_ptr<tcp_connection_t> tcp_connection_t_ptr;

			typedef command_type cmd_t;

		protected:
			tcp_connection_t_ptr m_connection;
			context_ptr m_context;

			libnetwrk::net::common::tsdeque<owned_message_t> m_incoming_messages;

			std::thread m_context_thread;
			std::thread m_process_messages_thread;

			bool m_connected = false;

		public:
			tcp_client() {
				LIBNETWRK_STATIC_ASSERT_OR_THROW(std::is_enum<command_type>::value,
					"client command_type template arg can only be an enum");

				try {
					m_context = std::make_shared<asio::io_context>(1);
				}
				catch (const std::exception& e) {
					LIBNETWRK_ERROR("failed to create tcp_client | %s", e.what());
				}
			}

			~tcp_client() {
				stop();
			}

			/// <summary>
			/// Client status
			/// </summary>
			/// <returns>true if connected, false if disconnected</returns>
			bool connected() {
				return m_connected;
			}

			/// <summary>
			/// Connect to TCP server
			/// </summary>
			/// <param name="host">: IPv4 address</param>
			/// <param name="port">: port</param>
			/// <returns>true if connected, false if not</returns>
			bool connect(const char* host, const unsigned short port) {
				return _connect(host, port);
			}

			/// <summary>
			/// Stop the client and clean up
			/// </summary>
			void stop() {
				m_connected = false;

				if (m_context)
					if (!m_context->stopped())
						m_context->stop();

				if (m_connection)
					if (m_connection->is_alive())
						m_connection->stop();

				m_incoming_messages.cancel_wait();

				if (m_context_thread.joinable())
					m_context_thread.join();

				if (m_process_messages_thread.joinable())
					m_process_messages_thread.join();

				LIBNETWRK_INFO("tcp_client stopped");
			}

			/// <summary>
			/// Processes a single message if the queue is not empty.
			/// </summary>
			/// <returns>true if a message has been processed, false if it hasn't</returns>
			bool process_single_message() {
				try {
					if (m_incoming_messages.empty())
						return false;

					message_t msg = m_incoming_messages.pop_front().m_msg;
					on_message(msg);
				}
				catch (const std::exception& e) {
					LIBNETWRK_ERROR("process_single_message() fail | %s", e.what());
					return false;
				}
				catch (...) {
					LIBNETWRK_ERROR("process_single_message() fail | undefined reason");
					return false;
				}

				return true;
			}

			/// <summary>
			/// Process messages while client is connected. This is a blocking function.
			/// </summary>
			void process_messages() {
				_process_messages();
			}

			/// <summary>
			/// Process messages while client is connected. 
			/// This function runs asynchronously until the client stops.
			/// </summary>
			void process_messages_async() {
				m_process_messages_thread = std::thread([&] { _process_messages(); });
			}

			/// <summary>
			/// Send a message
			/// </summary>
			/// <param name="message">: message to send</param>
			void send(message_t& message) {
				if (m_connection && m_connected) {
					if (m_connection->is_alive()) {
						m_connection->send(std::make_shared<message_t>(std::move(message)));
					}
					else {
						on_disconnect();
						stop();
					}
				}
			}

		protected:
			virtual void on_message(message_t& msg) { }

			virtual void on_disconnect() {}

		private:
			bool _connect(const char* host, const unsigned short port) {
				if (m_connected)
					return false;

				try {
					// Create ASIO endpoint
					asio::ip::tcp::endpoint ep(asio::ip::address::from_string(host), port);

					// Create ASIO socket
					asio::ip::tcp::socket socket(*m_context, ep.protocol());

					// Connect
					socket.connect(ep);

					// Create connection object
					m_connection = 
						std::make_shared<tcp_connection_t>(libnetwrk::net::common::connection_owner::client,
							std::move(socket), m_context, m_incoming_messages);

					// Start receiving messages
					m_connection->start();

					// Start ASIO context
					m_context_thread = std::thread([&] { m_context->run(); });

					m_connected = true;

					LIBNETWRK_INFO("connected to %s:%d", host, port);
				}
				catch (const std::exception& e) {
					LIBNETWRK_ERROR("failed to connect | %s", e.what());
					stop();
					return false;
				}
				catch (...) {
					LIBNETWRK_ERROR("failed to connect | fatal error");
					stop();
					return false;
				}

				return true;
			}

			void _process_messages(size_t max_messages = -1) {
				while (m_connected) {
					m_incoming_messages.wait();

					try {
						size_t message_count = 0;
						while (message_count < max_messages && !m_incoming_messages.empty()) {
							message_t msg = m_incoming_messages.pop_front().m_msg;
							on_message(msg);
							message_count++;
						}
					}
					catch (const std::exception& e) {
						LIBNETWRK_ERROR("do_process_messages() fail | %s", e.what());
					}
					catch (...) {
						LIBNETWRK_ERROR("do_process_messages() fail | undefined reason");
					}
				}
			}
	};
}

#endif