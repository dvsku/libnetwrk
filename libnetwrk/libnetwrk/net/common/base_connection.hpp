#ifndef LIBNETWRK_NET_COMMON_BASE_CONNECTION_HPP
#define LIBNETWRK_NET_COMMON_BASE_CONNECTION_HPP

#include <random>

#include "libnetwrk/net/definitions.hpp"
#include "libnetwrk/net/common/base_context.hpp"
#include "libnetwrk/net/common/containers/tsdeque.hpp"
#include "libnetwrk/net/message.hpp"
#include "libnetwrk/net/common/serialization/serializers/binary_serializer.hpp"

namespace libnetwrk::net::common {
	template <typename command_type, 
		typename serializer,
		typename storage = libnetwrk::nothing>
	class base_connection 
		: public std::enable_shared_from_this<base_connection<command_type, serializer, storage>>
	{
		public:
			typedef message<command_type, serializer>					message_t;
			typedef std::shared_ptr<message_t>							message_t_ptr;
			typedef owned_message<command_type, serializer, storage>	owned_message_t;

			typedef base_context<command_type, serializer, storage> base_context_t;

		protected:
			tsdeque<message_t_ptr> m_outgoing_messages;

			storage m_storage;

			message_t m_temp_message;

			uint64_t m_id;

			uint32_t m_verification_ans = 0;		// Correct verification answer, server only
			uint32_t m_verification_code = 0;

			base_context_t& m_parent_context;

		public:
			base_connection(base_context_t& parent_context) 
				: m_parent_context(parent_context) {}

			/// <summary>
			/// Get connection storage
			/// </summary>
			/// <returns>storage</returns>
			storage& get_storage() {
				return m_storage;
			}

			/// <summary>
			/// Get connection id
			/// </summary>
			uint64_t& id() {
				return m_id;
			}

			/// <summary>
			/// Start reading connection messages 
			/// </summary>
			void start() {
				if (m_parent_context.m_owner == connection_owner::server) {
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

			virtual void stop() {};

			virtual bool is_alive() = 0;

			/// <summary>
			/// Send message
			/// </summary>
			/// <param name="message">ptr to message</param>
			void send(const message_t_ptr& message) {
				asio::post(*m_parent_context.m_context, [this, message]() {
					bool was_empty = m_outgoing_messages.empty();
					m_outgoing_messages.push_back(message);

					if (was_empty)
						write_message_head();
				});
			}

			/// <summary>
			/// Send message. 
			/// Message object after sending should be considered in an undefined state and
			/// shouldn't be used further without reassigning.
			/// </summary>
			/// <param name="message">: message to send</param>
			void send(message_t& message) {
				send(std::make_shared<message_t>(std::move(message)));
			}

		protected:
			virtual void read_verification_message() = 0;

			virtual void read_message_head() = 0;

			virtual void read_message_body() = 0;

			virtual void write_verification_message() = 0;

			virtual void write_message_head() = 0;

			virtual void write_message_body() = 0;

			

			void read_verification_message_callback(std::error_code ec, std::size_t len) {
				if (!ec) {
					if (m_parent_context.m_owner == connection_owner::server) {
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
					m_temp_message.m_head.deserialize(m_temp_message.m_head_data);

					m_temp_message.m_head.m_receive_timestamp =
						std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

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
					if (m_parent_context.m_owner == connection_owner::server)
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
					if (m_outgoing_messages.front()->m_data.size() > 0) {
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

				if (m_parent_context.m_owner == connection_owner::server)
					owned_message.m_client = this->shared_from_this();
				else
					owned_message.m_client = nullptr;

				m_parent_context.m_incoming_messages.push_back(owned_message);

				read_message_head();
			}

		private:
			void on_disconnect() {
				stop();
			}

			void on_error(std::error_code ec) {
				stop();
				LIBNETWRK_ERROR("failed during read/write | %s", ec.message().c_str());
			}

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