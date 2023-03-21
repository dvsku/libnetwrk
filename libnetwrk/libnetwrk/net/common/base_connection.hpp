#ifndef LIBNETWRK_NET_COMMON_BASE_CONNECTION_HPP
#define LIBNETWRK_NET_COMMON_BASE_CONNECTION_HPP

#include <random>

#include "libnetwrk/net/definitions.hpp"
#include "libnetwrk/net/common/containers/tsdeque.hpp"
#include "libnetwrk/net/message.hpp"
#include "libnetwrk/net/common/serialization/serializers/binary_serializer.hpp"

namespace libnetwrk::net::common {
	enum class connection_owner : unsigned int {
		server, client
	};

	template <typename command_type, 
		typename serializer,
		typename storage = libnetwrk::nothing>
	class base_connection 
		: public std::enable_shared_from_this<base_connection<command_type, serializer, storage>>
	{
		public:
			typedef message<command_type, serializer> message_t;
			typedef owned_message<command_type, serializer, storage> owned_message_t;

		protected:
			context_ptr m_context;

			tsdeque<owned_message_t>& m_incoming_messages;
			tsdeque<message_t> m_outgoing_messages;

			connection_owner m_owner;
			storage m_storage;

			message_t m_temp_message;

			uint32_t m_verification_ans;		// Correct verification answer, server only
			uint32_t m_verification_code;

		public:
			base_connection(connection_owner owner, context_ptr context, tsdeque<owned_message_t>& queue)
				: m_incoming_messages(queue)
			{
				m_context = context;
				m_owner = owner;
				m_verification_ans = 0;
				m_verification_code = 0;
			};

			storage& get_storage() {
				return m_storage;
			}

			void start() {
				if (m_owner == connection_owner::server) {
					m_verification_code = generate_verification_code();
					m_verification_ans	= generate_verification_answer(m_verification_code);
					write_verification_message();
				}
				else {
					read_verification_message();
				}
			}

			virtual const std::string remote_address() = 0;

			virtual const unsigned short remote_port() = 0;

			virtual void stop() = 0;

			virtual bool is_alive() = 0;

			void send(const message_t& msg) {
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
			virtual void read_verification_message() = 0;

			virtual void read_message_head() = 0;

			virtual void read_message_body() = 0;

			virtual void write_verification_message() = 0;

			virtual void write_message_head() = 0;

			virtual void write_message_body() = 0;

			virtual void on_disconnect() {
				LIBNETWRK_INFO("client disconnected");
			}

			virtual void on_error(std::error_code ec) {
				LIBNETWRK_ERROR("failed during read/write | %s", ec.message().c_str());
			}

			void read_verification_message_callback(std::error_code ec, std::size_t len) {
				if (!ec) {
					if (m_owner == connection_owner::server) {
						if (m_verification_code == m_verification_ans) {
							read_message_head();
						}
						else {
							LIBNETWRK_WARNING("client verification failed, disconnecting");
							stop();
						}
					}
					else {
						m_verification_code = generate_verification_answer(m_verification_code);
						write_verification_message();
					}
				}
				else if (ec == asio::error::eof || ec == asio::error::connection_reset)
					on_disconnect();
				else
					on_error(ec);
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

			void write_verification_message_callback(std::error_code ec, std::size_t len) {
				if (!ec) {
					if (m_owner == connection_owner::server)
						read_verification_message();
					else
						read_message_head();
				}
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
				owned_message_t owned_message;
				owned_message.m_msg = m_temp_message;

				if (m_owner == connection_owner::server)
					owned_message.m_client = this->shared_from_this();
				else
					owned_message.m_client = nullptr;

				m_incoming_messages.push_back(owned_message);

				read_message_head();
			}

		private:
			uint32_t generate_verification_code() {
				std::random_device seed;
				std::default_random_engine generator(seed());
				std::uniform_int_distribution<uint32_t> distribution(0x0000, 0xFFFF);
				return distribution(generator);
			}

			uint32_t generate_verification_answer(uint32_t request) {
				return request ^ 21205;
			}
	};
}

#endif