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
			typedef libnetwrk::net::common::base_server<command_type, serializer, storage> base_t;

			typedef base_t::message_t			message_t;
			typedef base_t::owned_message_t		owned_message_t;
			typedef command_type				cmd_t;

			typedef libnetwrk::net::common::base_connection<command_type, serializer, storage> base_connection_t;
			typedef std::shared_ptr<base_connection_t> base_connection_t_ptr;
			typedef base_connection_t_ptr client_ptr;	

		private:
			typedef tcp_connection<command_type, serializer, storage> tcp_connection_t;
			typedef std::shared_ptr<tcp_connection_t> tcp_connection_t_ptr;			

		protected:
			acceptor_ptr m_acceptor;

		public:
			tcp_server(const std::string& name = "tcp server") : base_t(name) {};

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

				base_t::stop();
			}

		protected:
			virtual void on_message(owned_message_t& msg) override {}

			virtual bool on_before_client_connect(client_ptr client) {
				return true;
			}

			virtual void on_client_connect(client_ptr client) {}

			virtual void on_client_disconnect(client_ptr client) override {
				base_t::on_client_disconnect(client);
			}

			bool _start(const char* host, const unsigned short port) override {
				try {
					// Create ASIO context
					this->m_context = std::make_shared<asio::io_context>(1);

					// Create ASIO acceptor
					m_acceptor = std::make_shared<asio::ip::tcp::acceptor>
						(*(this->m_context), asio::ip::tcp::endpoint(asio::ip::address::from_string(host), port));

					// Start listening for and accepting connections
					_accept();

					// Start ASIO context
					this->start_context();

					this->m_running = true;

					LIBNETWRK_INFO(this->name(), "listening for connections on {}:{}", host, port);
				}
				catch (const std::exception& e) {
					LIBNETWRK_ERROR(this->name(), "failed to start listening | {}", e.what());
					stop();
					return false;
				}
				catch (...) {
					LIBNETWRK_ERROR(this->name(), "failed to start listening | fatal error");
					stop();
					return false;
				}

				return true;
			}

			void _accept() override {
				m_acceptor->async_accept(
					[this](std::error_code ec, asio::ip::tcp::socket socket) {
						if (!ec) {
							LIBNETWRK_VERBOSE(this->name(), "attempted connection from {}:{}",
								socket.remote_endpoint().address().to_string().c_str(),
								socket.remote_endpoint().port());

							tcp_connection_t_ptr new_connection =
								std::make_shared<tcp_connection_t>(*this, std::move(socket));

							if (on_before_client_connect(new_connection)) {
								libnetwrk_guard guard(this->m_connections_mutex);

								this->m_connections.push_back(new_connection);
								this->m_connections.back()->id() = ++this->m_id_counter;
								this->m_connections.back()->start();
								on_client_connect(new_connection);

								LIBNETWRK_INFO(this->name(), "connection success from {}:{}",
									this->m_connections.back()->remote_address().c_str(),
									this->m_connections.back()->remote_port());
							}
							else {
								LIBNETWRK_WARNING(this->name(), "connection denied");
							}
						}
						else if (ec == asio::error::operation_aborted) {
							LIBNETWRK_INFO(this->name(), "listening stopped");
							return;
						}
						else {
							LIBNETWRK_ERROR(this->name(), "failed to accept connection | {}", ec.message().c_str());
						}

						_accept();
					}
				);
			};
	};
}

#endif