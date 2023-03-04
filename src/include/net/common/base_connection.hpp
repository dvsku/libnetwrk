#ifndef LIBNETWRK_NET_COMMON_BASE_CONNECTION_HPP
#define LIBNETWRK_NET_COMMON_BASE_CONNECTION_HPP

#include "lib/asio/asio.hpp"

typedef std::shared_ptr<asio::io_context> context_ptr;
typedef std::shared_ptr<asio::ip::tcp::acceptor> acceptor_ptr;

#include "net/message.hpp"
#include "net/common/containers/tsdeque.hpp"

namespace libnetwrk::net::common {
	// Empty struct, used for default connection->connection_data
	struct nothing {};

	// Connection owner
	enum class owner { server, client };

	template <typename command_type, typename storage = nothing>
	class base_connection : public std::enable_shared_from_this<base_connection<command_type, storage>> {
		protected:
			context_ptr m_context;

			tsdeque<owned_message<command_type, storage>>& m_incoming_messages;
			tsdeque<message<command_type>> m_outgoing_messages;

			owner m_owner;
			storage m_storage;

			message<command_type> m_temp_message;

		public:
			base_connection(owner owner, context_ptr context,
				tsdeque<owned_message<command_type, storage>>& queue) 
				: m_incoming_messages(queue)
			{
				m_context = context;
				m_owner = owner;
			};

			storage& get_storage() {
				return m_storage;
			}

			void start() {
				read_message_head();
			}

			virtual const std::string remote_address() = 0;

			virtual const unsigned short remote_port() = 0;

			virtual void stop() = 0;

			virtual bool is_alive() = 0;

			void send(const message<command_type>& msg) {
				asio::post(*m_context,
					[this, msg]() {
						bool was_empty = m_outgoing_messages.empty();
						m_outgoing_messages.push_back(msg);

						if (was_empty)
							write_message_head();
					}
				);
			}

		protected:
			virtual void read_message_head() = 0;

			virtual void read_message_body() = 0;

			virtual void write_message_head() = 0;

			virtual void write_message_body() = 0;

			virtual void on_disconnect() {
				//OUTPUT_INFO("client disconnected");
			}

			virtual void on_error(std::error_code ec) {
				//OUTPUT_ERROR("failed during read/write | %s", ec.message().c_str());
			}

			void read_message_head_callback(std::error_code ec, std::size_t len) {
				if (!ec) {
					// MESSAGE HAS A BODY
					if (m_temp_message.m_head.m_data_len > 0) {
						m_temp_message.m_data.resize(m_temp_message.m_head.m_data_len);
						read_message_body();
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

			void read_message_body_callback(std::error_code ec, std::size_t len) {
				if (!ec)
					add_message_to_queue();
				else if (ec == asio::error::eof || ec == asio::error::connection_reset)
					on_disconnect();
				else
					on_error(ec);
			}

			void write_message_head_callback(std::error_code ec, std::size_t len) {
				if (!ec) {
					if (m_outgoing_messages.front().m_data.size() > 0) {
						write_message_body();
					}
					else {
						m_outgoing_messages.pop_front();

						if (!m_outgoing_messages.empty())
							write_message_head();
					}
				}
				else if (ec == asio::error::eof || ec == asio::error::connection_reset)
					on_disconnect();
				else
					on_error(ec);
			}

			void write_message_body_callback(std::error_code ec, std::size_t len) {
				if (!ec) {
					m_outgoing_messages.pop_front();

					if (!m_outgoing_messages.empty())
						write_message_head();
				}
				else if (ec == asio::error::eof || ec == asio::error::connection_reset)
					on_disconnect();
				else
					on_error(ec);
			}

			void add_message_to_queue() {
				owned_message<command_type, storage> owned_message;
				owned_message.m_msg = m_temp_message;

				if (m_owner == owner::server)
					owned_message.m_client = this->shared_from_this();
				else
					owned_message.m_client = nullptr;

				m_incoming_messages.push_back(owned_message);

				read_message_head();
			}
	};

	template <typename command_type, typename storage = nothing> 
	using base_connection_ptr = std::shared_ptr<base_connection<command_type, storage>>;
}

#endif