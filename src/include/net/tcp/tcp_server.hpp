#ifndef LIBNETWRK_NET_TCP_TCP_SERVER_HPP
#define LIBNETWRK_NET_TCP_TCP_SERVER_HPP

#include <thread>
#include <exception>

#include "net/message.hpp"
#include "net/common/containers/tsdeque.hpp"
#include "net/tcp/tcp_connection.hpp"
#include "net/definitions.hpp"

namespace libnetwrk::net::tcp {
	template <typename command_type, typename storage = libnetwrk::nothing>
	class tcp_server {
		protected:
			context_ptr m_context;
			acceptor_ptr m_acceptor;

			libnetwrk::net::common::tsdeque<libnetwrk::net::owned_message<command_type, storage>> m_incoming_messages;
			std::deque<std::shared_ptr<tcp_connection<command_type, storage>>> m_connections;

			std::thread m_asio_thread;
			std::thread m_update_thread;

			bool m_running = false;

		public:
			tcp_server() {
				try {
					m_context = std::make_shared<asio::io_context>(1);
				}
				catch (const std::exception& e) {
					VAR_IGNORE(e);
					//OUTPUT_ERROR("failed to create tcp_service | %s", e.what());
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

					//OUTPUT_INFO("listening for connections on %s:%d", host, port);

					// Start processing received messages
					if (process_messages)
						do_process_messages();
				}
				catch (const std::exception& e) {
					//OUTPUT_ERROR("failed to start listening | %s", e.what());
					stop();
					return false;
				}
				catch (...) {
					//OUTPUT_ERROR("failed to start listening | fatal error");
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

					//OUTPUT_INFO("listening for connections on %s:%d", host, port);

					// Start processing received messages
					if (process_messages)
						m_update_thread = std::thread([&] { do_process_messages(); });
				}
				catch (const std::exception& e) {
					VAR_IGNORE(e);
					//OUTPUT_ERROR("failed to start listening | %s", e.what());
					stop();
					return false;
				}
				catch (...) {
					//OUTPUT_ERROR("failed to start listening | fatal error");
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

				//OUTPUT_INFO("tcp service stopped");
			}

			/// <summary>
			/// Processes a single message if the queue is not empty.
			/// </summary>
			/// <returns>true if a message has been processed, false if it hasn't</returns>
			bool process_single_message() {
				try {
					if (m_incoming_messages.empty())
						return false;
					
					libnetwrk::net::owned_message<command_type, storage> msg = m_incoming_messages.pop_front();
					on_message(msg);
				}
				catch (const std::exception& e) {
					//OUTPUT_ERROR("update_one() fail | %s", e.what());
					return false;
				}
				catch (...) {
					//OUTPUT_ERROR("update_one() fail | undefined reason");
					return false;
				}

				return true;
			}

			void queue_async_job(std::function<void()> const& lambda) {
				asio::post(*(this->m_context), lambda);
			}

			void send(tcp_connection<command_type, storage> client,
				const libnetwrk::net::message<command_type>& msg) 
			{
				if (client && client->is_alive()) {
					client->send(msg);
				}
				else {
					on_client_disconnect(client);
					client.reset();

					m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), client), m_connections.end());
				}
			}

			void send_all(const libnetwrk::net::message<command_type>& msg) {
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
					m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), nullptr), m_connections.end());
			}

			void send_all(const libnetwrk::net::message<command_type>& msg, 
				std::function<bool(const storage&)> predicate) 
			{
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
					m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), nullptr), m_connections.end());
			}

		protected:
			virtual void on_message(libnetwrk::net::owned_message<command_type, storage>& msg) {
				return;
			}

			virtual bool on_client_connect(tcp_connection_ptr<command_type, storage> client) {
				return true;
			}

			virtual void on_client_disconnect(tcp_connection_ptr<command_type, storage> client) {
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
							/*OUTPUT_INFO("attempted connection from %s:%d", socket.remote_endpoint().address().to_string().c_str(),
								socket.remote_endpoint().port());*/

							tcp_connection_ptr<command_type, storage> new_connection =
								std::make_shared<tcp_connection<command_type, storage>>(
									libnetwrk::net::common::base_connection<command_type, storage>::owner::server,
									std::move(socket), this->context(), m_incoming_messages);

							if (on_client_connect(new_connection)) {
								m_connections.push_back(new_connection);
								m_connections.back()->start();

								/*OUTPUT_INFO("connection success from %s:%d", m_connections.back()->get_remote_address().c_str(),
									m_connections.back()->get_remote_port());*/
							}
							else {
								/*OUTPUT_WARNING("connection denied");*/
							}
						}
						else if (ec == asio::error::operation_aborted) {
							/*OUTPUT_INFO("listening stopped");*/
							return;
						}
						else {
							/*OUTPUT_ERROR("failed to accept connection | %s", ec.message().c_str());*/
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
						libnetwrk::net::owned_message<command_type, storage> msg = m_incoming_messages.pop_front();
						on_message(msg);
						message_count++;
					}
				}
			}
	};
}

#endif