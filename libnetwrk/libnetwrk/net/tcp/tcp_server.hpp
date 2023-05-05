#ifndef LIBNETWRK_NET_TCP_TCP_SERVER_HPP
#define LIBNETWRK_NET_TCP_TCP_SERVER_HPP

#include <thread>
#include <exception>

#include "libnetwrk/net/definitions.hpp"
#include "libnetwrk/net/common/base_server.hpp"
#include "libnetwrk/net/tcp/tcp_connection.hpp"

namespace libnetwrk::net::tcp {
	template <typename command_type, 
		typename serializer = libnetwrk::net::common::binary_serializer,
		typename storage = libnetwrk::nothing>
	class tcp_server 
		: public libnetwrk::net::common::base_server<command_type, serializer, storage> 
	{
		public:
			typedef libnetwrk::net::message<command_type, serializer> message_t;
			typedef libnetwrk::net::owned_message<command_type, serializer, storage> owned_message_t;

			typedef libnetwrk::net::common::base_connection<command_type, serializer, storage> base_connection_t;
			typedef std::shared_ptr<base_connection_t> base_connection_t_ptr;
			typedef base_connection_t_ptr client_ptr;

			typedef command_type cmd_t;

		private:
			typedef tcp_connection<command_type, serializer, storage> tcp_connection_t;
			typedef std::shared_ptr<tcp_connection_t> tcp_connection_t_ptr;
			typedef libnetwrk::net::common::base_server<command_type, serializer, storage> base;

		protected:
			acceptor_ptr m_acceptor;

		public:
			tcp_server(const std::string& name = "tcp server") 
				: libnetwrk::net::common::base_server<command_type, serializer, storage>(name) {};

			virtual ~tcp_server() {
				stop();
			}

			/// <summary>
			/// Stop server
			/// </summary>
			void stop() override {
				if (m_acceptor)
					if (m_acceptor->is_open())
						m_acceptor->close();

				base::stop();
			}

		protected:
			virtual void on_message(owned_message_t& msg) override {}

			virtual bool on_before_client_connect(client_ptr client) {
				return true;
			}

			virtual void on_client_connect(client_ptr client) {}

			virtual void on_client_disconnect(client_ptr client) override {
				base::on_client_disconnect(client);
			}

			bool _start(const char* host, const unsigned short port) override {
				try {
					// Create ASIO context
					base::m_context = std::make_shared<asio::io_context>(1);

					// Create ASIO acceptor
					m_acceptor = std::make_shared<asio::ip::tcp::acceptor>
						(*(base::m_context), asio::ip::tcp::endpoint(asio::ip::address::from_string(host), port));

					// Start listening for and accepting connections
					_accept();

					// Start ASIO context
					base::m_context_thread = std::thread([this] { base::m_context->run(); });

					base::m_running = true;

					LIBNETWRK_INFO("listening for connections on %s:%d", host, port);
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

			void _accept() override {
				m_acceptor->async_accept(
					[this](std::error_code ec, asio::ip::tcp::socket socket) {
						if (!ec) {
							LIBNETWRK_VERBOSE("attempted connection from %s:%d", 
								socket.remote_endpoint().address().to_string().c_str(),
								socket.remote_endpoint().port());

							tcp_connection_t_ptr new_connection =
								std::make_shared<tcp_connection_t>(*this, std::move(socket));

							if (on_before_client_connect(new_connection)) {
								libnetwrk_guard guard(base::m_connections_mutex);

								base::m_connections.push_back(new_connection);
								base::m_connections.back()->id() = ++base::m_id_counter;
								base::m_connections.back()->start();
								on_client_connect(new_connection);

								LIBNETWRK_INFO("connection success from %s:%d", 
									base::m_connections.back()->remote_address().c_str(),
									base::m_connections.back()->remote_port());
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

						_accept();
					}
				);
			};
	};
}

#endif