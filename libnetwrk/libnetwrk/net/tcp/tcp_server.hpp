#ifndef LIBNETWRK_NET_TCP_TCP_SERVER_HPP
#define LIBNETWRK_NET_TCP_TCP_SERVER_HPP

#include <thread>
#include <exception>

#include "libnetwrk/net/definitions.hpp"
#include "libnetwrk/net/message.hpp"
#include "libnetwrk/net/common/containers/tsdeque.hpp"
#include "libnetwrk/net/tcp/tcp_connection.hpp"
#include "libnetwrk/net/common/serialization/serializers/binary_serializer.hpp"

namespace libnetwrk::net::tcp {
	template <typename command_type, 
		typename serializer = libnetwrk::net::common::binary_serializer,
		typename storage = libnetwrk::nothing>
	class tcp_server {
		public:
			typedef libnetwrk::net::message<command_type, serializer> message_t;
			typedef libnetwrk::net::owned_message<command_type, serializer, storage> owned_message_t;

			typedef tcp_connection<command_type, serializer, storage> tcp_connection_t;
			typedef std::shared_ptr<tcp_connection_t> tcp_connection_t_ptr;

		protected:
			context_ptr m_context;
			acceptor_ptr m_acceptor;

			libnetwrk::net::common::tsdeque<owned_message_t> m_incoming_messages;
			
			std::deque<tcp_connection_t_ptr> m_connections;

			std::thread m_asio_thread;
			std::thread m_update_thread;

			bool m_running = false;

		public:
			tcp_server() {
				try {
					m_context = std::make_shared<asio::io_context>(1);
				}
				catch (const std::exception& e) {
					LIBNETWRK_ERROR("failed to create tcp_service | %s", e.what());
				}
			};

			~tcp_server() {
				stop();
			};

			context_ptr context() {
				return this->m_context;
			}

			bool running() {
				return m_running;
			}

			bool start(const char* host, const unsigned short port, const bool process_messages = true) {
				if (m_running)
					return false;

				try {
					// Create ASIO acceptor
					m_acceptor = std::make_shared<asio::ip::tcp::acceptor>
						(*m_context, asio::ip::tcp::endpoint(asio::ip::address::from_string(host), port));
					
					// Start listening for and accepting connections
					do_accept();

					// Start ASIO context
					m_asio_thread = std::thread([&] { start_context(); });

					m_running = true;

					LIBNETWRK_INFO("listening for connections on %s:%d", host, port);

					// Start processing received messages
					if (process_messages)
						do_process_messages();
				}
				catch (const std::exception& e) {
					LIBNETWRK_ERROR("failed to start listening | %s", e.what());
					stop();
					return false;
				}
				catch (...) {
					LIBNETWRK_ERROR("failed to start listening | fatal error");
					stop();
					return false;
				}

				return true;
			}

			bool start_async(const char* host, const unsigned short port, const bool process_messages = true) {
				if (m_running)
					return false;

				try {
					// Create ASIO acceptor
					m_acceptor = std::make_shared<asio::ip::tcp::acceptor>
						(*m_context, asio::ip::tcp::endpoint(asio::ip::address::from_string(host), port));

					// Start listening for and accepting connections
					do_accept();

					// Start ASIO context
					m_asio_thread = std::thread([&] { start_context(); });

					m_running = true;

					LIBNETWRK_INFO("listening for connections on %s:%d", host, port);

					// Start processing received messages
					if (process_messages)
						m_update_thread = std::thread([&] { do_process_messages(); });
				}
				catch (const std::exception& e) {
					LIBNETWRK_ERROR("failed to start listening | %s", e.what());
					stop();
					return false;
				}
				catch (...) {
					LIBNETWRK_ERROR("failed to start listening | fatal error");
					stop();
					return false;
				}

				return true;
			}

			void stop() {
				m_running = false;

				if (m_acceptor)
					if (m_acceptor->is_open())
						m_acceptor->close();

				if (m_context)
					if (!m_context->stopped())
						m_context->stop();

				if (m_asio_thread.joinable())
					m_asio_thread.join();

				m_incoming_messages.cancel_wait();

				if (m_update_thread.joinable())
					m_update_thread.join();

				LIBNETWRK_INFO("tcp service stopped");
			}

			/// <summary>
			/// Processes a single message if the queue is not empty.
			/// </summary>
			/// <returns>true if a message has been processed, false if it hasn't</returns>
			bool process_single_message() {
				try {
					if (m_incoming_messages.empty())
						return false;
					
					owned_message_t msg =
						m_incoming_messages.pop_front();
					
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

			void queue_async_job(std::function<void()> const& lambda) {
				asio::post(*(this->m_context), lambda);
			}

			void send(tcp_connection_t& client, const message_t& msg) {
				if (client && client->is_alive()) {
					client->send(msg);
				}
				else {
					on_client_disconnect(client);
					client.reset();

					m_connections.erase(std::remove(m_connections.begin(), 
						m_connections.end(), client), m_connections.end());
				}
			}

			void send_all(const message_t& msg) {
				bool has_invalid_clients = false;

				for (auto& client : m_connections) {
					if (client && client->is_alive()) {
						client->send(msg);
					}
					else {
						on_client_disconnect(client);
						client.reset();
						has_invalid_clients = true;
					}
				}

				if (has_invalid_clients)
					m_connections.erase(std::remove(m_connections.begin(),
						m_connections.end(), nullptr), m_connections.end());
			}

			void send_all(const message_t& msg, std::function<bool(const storage&)> predicate) {
				bool has_invalid_clients = false;

				for (auto& client : m_connections) {
					if (client && client->is_alive()) {
						if (predicate(client->connection_data()))
							client->send(msg);
					}
					else {
						on_client_disconnect(client);
						client.reset();
						has_invalid_clients = true;
					}
				}

				if (has_invalid_clients)
					m_connections.erase(std::remove(m_connections.begin(), 
						m_connections.end(), nullptr), m_connections.end());
			}

		protected:
			virtual void on_message(owned_message_t& msg) {
				return;
			}

			virtual bool on_client_connect(tcp_connection_t_ptr client) {
				return true;
			}

			virtual void on_client_disconnect(tcp_connection_t_ptr client) {
				return;
			}

		private:
			void start_context() {
				m_context->run();
			}

			void do_accept() {
				m_acceptor->async_accept(
					[this](std::error_code ec, asio::ip::tcp::socket socket) {
						if (!ec) {
							LIBNETWRK_VERBOSE("attempted connection from %s:%d", 
								socket.remote_endpoint().address().to_string().c_str(),
								socket.remote_endpoint().port());

							tcp_connection_t_ptr new_connection =
								std::make_shared<tcp_connection_t>(
									libnetwrk::net::common::connection_owner::server,
									std::move(socket), this->context(), m_incoming_messages);

							if (on_client_connect(new_connection)) {
								m_connections.push_back(new_connection);
								m_connections.back()->start();

								LIBNETWRK_INFO("connection success from %s:%d", 
									m_connections.back()->remote_address().c_str(),
									m_connections.back()->remote_port());
							}
							else {
								LIBNETWRK_WARNING("connection denied");
							}
						}
						else if (ec == asio::error::operation_aborted) {
							LIBNETWRK_INFO("listening stopped");
							return;
						}
						else {
							LIBNETWRK_ERROR("failed to accept connection | %s", ec.message().c_str());
						}

						do_accept();
					}
				);
			};

			void do_process_messages(size_t max_messages = -1) {
				while (m_running) {
					m_incoming_messages.wait();
					size_t message_count = 0;
					while (message_count < max_messages && !m_incoming_messages.empty()) {
						owned_message_t msg = m_incoming_messages.pop_front();
						on_message(msg);
						message_count++;
					}
				}
			}
	};
}

#endif