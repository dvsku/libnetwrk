#ifndef LIBNETWRK_TCP_TCP_CLIENT_H
#define LIBNETWRK_TCP_TCP_CLIENT_H

#include <thread>

#include "net/common/connection.hpp"
#include "net/common/message.hpp"
#include "net/common/containers/tsdeque.hpp"
#include "net/definitions.hpp"

namespace libnetwrk::net::tcp {
	template <typename command_type, typename storage = libnetwrk::net::common::nothing>
	class tcp_client {
		protected:
			libnetwrk::net::common::connection_ptr<command_type, storage> m_connection;
			context_ptr m_context;

			libnetwrk::net::common::tsdeque<libnetwrk::net::common::owned_message<command_type, storage>> m_incoming_messages;

			std::thread m_asio_thread;
			std::thread m_update_thread;

			bool m_running;

		public:
			tcp_client() : m_running(false) {
				try {
					m_context = std::make_shared<asio::io_context>(1);
				}
				catch (const std::exception& e) {
					VAR_IGNORE(e);
					//OUTPUT_ERROR("failed to create tcp_client | %s", e.what());
				}
			}

			~tcp_client() {
				stop();
			}

			bool running() {
				return m_running;
			}

			bool connect(const char* host, const unsigned short port) {
				try {
					asio::ip::tcp::endpoint ep(asio::ip::address::from_string(host), port);
					asio::ip::tcp::socket socket(*m_context, ep.protocol());
					socket.connect(ep);

					m_connection = std::make_shared<libnetwrk::net::common::connection<command_type>>(
						libnetwrk::net::common::owner::client, std::move(socket),
						m_context, m_incoming_messages);

					m_connection->start();

					m_asio_thread = std::thread([&] { m_context->run(); });

					m_running = true;

					//OUTPUT_INFO("connected to %s:%d", host, port);
				}
				catch (const std::exception& e) {
					VAR_IGNORE(e);
					//OUTPUT_ERROR("failed to connect | %s", e.what());
					return false;
				}

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

				//OUTPUT_INFO("tcp_client stopped");
			}

			void async_update() {
				m_update_thread = worker([&] { do_update; });
			}

			void update_one() {
				try {
					if (!m_incoming_messages.empty()) {
						libnetwrk::net::common::message<command_type> msg = m_incoming_messages.pop_front().m_msg;
						on_message(msg);
					}
				}
				catch (const std::exception& e) {
					//OUTPUT_ERROR("update_one() fail | %s", e.what());
				}
				catch (...) {
					//OUTPUT_ERROR("update_one() fail | undefined reason");
				}
			}

			void send(const libnetwrk::net::common::message<command_type>& message) {
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
			virtual void on_message(libnetwrk::net::common::message<command_type>& msg) { }

			virtual void on_disconnect() {}

		private:
			void do_update(size_t max_messages = -1) {
				while (m_running) {
					m_incoming_messages.wait();

					try {
						size_t message_count = 0;
						while (message_count < max_messages && !m_incoming_messages.empty()) {
							libnetwrk::net::common::message<command_type> msg = m_incoming_messages.pop_front().m_msg;
							on_message(msg);
							message_count++;
						}
					}
					catch (const std::exception& e) {
						//OUTPUT_ERROR("do_update() fail | %s", e.what());
					}
					catch (...) {
						//OUTPUT_ERROR("do_update() fail | undefined reason");
					}
				}
			}
	};
}

#endif