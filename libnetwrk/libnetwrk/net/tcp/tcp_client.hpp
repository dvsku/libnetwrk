#ifndef LIBNETWRK_NET_TCP_TCP_CLIENT_HPP
#define LIBNETWRK_NET_TCP_TCP_CLIENT_HPP

#include <thread>
#include <exception>

#include "libnetwrk/net/message.hpp"
#include "libnetwrk/net/common/containers/tsdeque.hpp"
#include "libnetwrk/net/tcp/tcp_connection.hpp"
#include "libnetwrk/net/definitions.hpp"

namespace libnetwrk::net::tcp {
	template <typename command_type, 
		typename serializer = libnetwrk::net::common::binary_serializer,
		typename storage = libnetwrk::nothing>
	class tcp_client {
		protected:
			std::shared_ptr<tcp_connection<command_type, serializer, storage>> m_connection;
			context_ptr m_context;

			libnetwrk::net::common::tsdeque<libnetwrk::net::owned_message<command_type, 
				serializer, storage>> m_incoming_messages;

			std::thread m_asio_thread;
			std::thread m_update_thread;

			bool m_running = false;

		public:
			tcp_client() {
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

			bool running() {
				return m_running;
			}

			bool connect(const char* host, const unsigned short port, const bool process_messages = true) {
				if (!_connect(host, port)) return false;

				// Start processing received messages
				if (process_messages)
					do_process_messages();

				return true;
			}

			bool connect_async(const char* host, const unsigned short port, const bool process_messages = true) {
				if (!_connect(host, port)) return false;

				// Start processing received messages
				if (process_messages)
					m_update_thread = std::thread([&] { do_process_messages(); });

				return true;
			}

			void stop() {
				m_running = false;

				if (m_context)
					if (!m_context->stopped())
						m_context->stop();

				if (m_connection)
					if (m_connection->is_alive())
						m_connection->stop();

				m_incoming_messages.cancel_wait();

				if (m_asio_thread.joinable())
					m_asio_thread.join();

				if (m_update_thread.joinable())
					m_update_thread.join();

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

					libnetwrk::net::message<command_type, serializer> msg = m_incoming_messages.pop_front().m_msg;
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

			void send(const libnetwrk::net::message<command_type, serializer>& message) {
				if (m_connection != nullptr && m_running) {
					if (m_connection->is_alive()) {
						m_connection->send(message);
					}
					else {
						on_disconnect();
						stop();
					}
				}
			}

		protected:
			virtual void on_message(libnetwrk::net::message<command_type, serializer>& msg) { }

			virtual void on_disconnect() {}

		private:
			bool _connect(const char* host, const unsigned short port) {
				if (m_running)
					return false;

				try {
					// Create ASIO endpoint
					asio::ip::tcp::endpoint ep(asio::ip::address::from_string(host), port);

					// Create ASIO socket
					asio::ip::tcp::socket socket(*m_context, ep.protocol());

					// Connect
					socket.connect(ep);

					// Create connection object
					m_connection = std::make_shared<tcp_connection<command_type, serializer, storage>>(
						libnetwrk::net::common::connection_owner::client,
						std::move(socket), m_context, m_incoming_messages);

					// Start receiving messages
					m_connection->start();

					// Start ASIO context
					m_asio_thread = std::thread([&] { m_context->run(); });

					m_running = true;

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

			void do_process_messages(size_t max_messages = -1) {
				while (m_running) {
					m_incoming_messages.wait();

					try {
						size_t message_count = 0;
						while (message_count < max_messages && !m_incoming_messages.empty()) {
							libnetwrk::net::message<command_type, serializer> msg = 
								m_incoming_messages.pop_front().m_msg;
							
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