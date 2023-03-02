#ifndef LIBNETWRK_NET_COMMON_CONNECTION_H
#define LIBNETWRK_NET_COMMON_CONNECTION_H

#include "lib/asio/asio.hpp"

typedef std::shared_ptr<asio::io_context> context_ptr;
typedef std::shared_ptr<asio::ip::tcp::acceptor> acceptor_ptr;

#include "net/common/message.hpp"
#include "net/common/containers/tsdeque.hpp"

namespace libnetwrk::net::common {
	// Empty struct, used for default connection->connection_data
	struct nothing {};

	// Connection owner
	enum class owner { server, client };

	template <typename command_type, typename storage = nothing>
	class connection : public std::enable_shared_from_this<connection<command_type, storage>> {
		protected:
			context_ptr m_context;
			asio::ip::tcp::socket m_socket;

			tsdeque<owned_message<command_type, storage>>& m_incoming_messages;
			tsdeque<message<command_type>> m_outgoing_messages;

			owner m_owner;
			storage m_storage;

			message<command_type> m_temp_message;

		public:
			connection(owner owner, asio::ip::tcp::socket socket, context_ptr context, 
				tsdeque<owned_message<command_type, storage>>& queue) 
				: m_socket(std::move(socket)), m_incoming_messages(queue)
			{
				m_context = context;
				m_owner = owner;
			};

			asio::ip::tcp::socket& socket() {
				return m_socket;
			}

			storage& get_storage() {
				return m_storage;
			}

			const std::string remote_address() {
				return m_socket.remote_endpoint().address().to_string();
			}

			const unsigned short remote_port() {
				return m_socket.remote_endpoint().port();
			}

			void start() {
				do_read_header();
			}

			void stop() {
				m_socket.close();
			}

			bool is_alive() {
				return m_socket.is_open();
			}

			void send(const message<command_type>& msg) {
				post(*m_context,
					[this, msg]() {
						bool was_empty = m_outgoing_messages.is_empty();
						m_outgoing_messages.push_back(msg);

						if (was_empty)
							do_write_header();
					}
				);
			}

		private:
			void do_read_header() {
				async_read(m_socket, asio::buffer(&m_temp_message.m_head, sizeof(message_head<command_type>)),
					[this](std::error_code ec, std::size_t len) {
						if (!ec) {
							// MESSAGE HAS A BODY
							if (m_temp_message.m_head.m_data_len > 0) {
								m_temp_message.m_data.resize(m_temp_message.m_head.m_data_len);
								do_read_body();
							}
							else {	// MESSAGE HAS NO BODY
								add_message_to_queue();
							}
						}
						else if (ec == asio::error::eof || ec == asio::error::connection_reset)
							on_disconnect();
						else
							on_error(ec);
					}
				);
			}

			void do_read_body() {
				async_read(m_socket, asio::buffer(m_temp_message.m_data.data(), m_temp_message.m_head.m_data_len),
					[this](std::error_code ec, std::size_t len) {
						if (!ec)
							add_message_to_queue();
						else if (ec == asio::error::eof || ec == asio::error::connection_reset)
							on_disconnect();
						else
							on_error(ec);
					}
				);
			}

			void do_write_header() {
				async_write(m_socket, asio::buffer(&m_outgoing_messages.front().m_head, sizeof(message_head<command_type>)),
					[this](std::error_code ec, std::size_t len) {
						if (!ec) {
							if (m_outgoing_messages.front().m_data.size() > 0) {
								do_write_body();
							}
							else {
								m_outgoing_messages.pop_front();

								if (!m_outgoing_messages.is_empty())
									do_write_header();
							}
						}
						else if (ec == asio::error::eof || ec == asio::error::connection_reset)
							on_disconnect();
						else
							on_error(ec);
					}
				);
			}

			void do_write_body() {
				async_write(m_socket, asio::buffer(m_outgoing_messages.front().m_data.data(),
					m_outgoing_messages.front().m_data.size()),
					[this](std::error_code ec, std::size_t len) {
						if (!ec) {
							m_outgoing_messages.pop_front();

							if (!m_outgoing_messages.is_empty())
								do_write_header();
						}
						else if (ec == asio::error::eof || ec == asio::error::connection_reset)
							on_disconnect();
						else
							on_error(ec);
					}
				);
			}

			void add_message_to_queue() {
				owned_message<command_type, storage> owned_message;
				owned_message.set_message(m_temp_message);

				if (m_owner == owner::server)
					owned_message.set_client(this->shared_from_this());
				else
					owned_message.set_client(nullptr);

				m_incoming_messages.push_back(owned_message);

				do_read_header();
			}

			void on_disconnect() {
				m_socket.close();
				//OUTPUT_INFO("client disconnected");
			}

			void on_error(std::error_code ec) {
				m_socket.close();
				//OUTPUT_ERROR("failed during read/write | %s", ec.message().c_str());
			}
	};

	template <typename command_type, typename connection_data> 
	using connection_ptr = std::shared_ptr<connection<command_type, connection_data>>;
}

#endif