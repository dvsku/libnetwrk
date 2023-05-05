#ifndef LIBNETWRK_NET_COMMON_BASE_CLIENT_HPP
#define LIBNETWRK_NET_COMMON_BASE_CLIENT_HPP

#include "libnetwrk/net/definitions.hpp"
#include "libnetwrk/net/message.hpp"
#include "libnetwrk/net/macros.hpp"
#include "libnetwrk/net/common/base_context.hpp"
#include "libnetwrk/net/common/base_connection.hpp"
#include "libnetwrk/net/common/serialization/serializers/binary_serializer.hpp"
#include "libnetwrk/net/common/containers/tsdeque.hpp"

namespace libnetwrk::net::common {
	template <typename command_type,
		typename serializer = libnetwrk::net::common::binary_serializer,
		typename storage = libnetwrk::nothing>
	class base_client : public base_context<command_type, serializer, storage> {
		public:
			typedef libnetwrk::net::message<command_type, serializer>					message_t;
			typedef std::shared_ptr<message_t>											message_t_ptr;
			typedef libnetwrk::net::owned_message<command_type, serializer, storage>	owned_message_t;

			typedef base_context<command_type, serializer, storage>			base_context_t;
			typedef base_connection<command_type, serializer, storage>		base_connection_t;
			typedef std::shared_ptr<base_connection_t>						base_connection_t_ptr;

		protected:
			bool m_connected = false;
			base_connection_t_ptr m_connection;

		private:
			std::thread m_context_thread;
			std::thread m_process_messages_thread;

		public:
			base_client(const std::string& name = "base client") : base_context_t(name, connection_owner::client) {
				LIBNETWRK_STATIC_ASSERT_OR_THROW(std::is_enum<command_type>::value,
					"client command_type template arg can only be an enum");
			}

			virtual ~base_client() {
				disconnect();
			}

			/// <summary>
			/// Client status
			/// </summary>
			/// <returns>true if connected, false if disconnected</returns>
			bool connected() {
				return m_connected;
			}

			/// <summary>
			/// Connect to TCP server
			/// </summary>
			/// <param name="host">: IPv4 address</param>
			/// <param name="port">: port</param>
			/// <returns>true if connected, false if not</returns>
			bool connect(const char* host, const unsigned short port) {
				if (m_connected) return false;
				return _connect(host, port);
			}

			/// <summary>
			/// Disconnect the client and clean up
			/// </summary>
			void disconnect() {
				if (!m_connected) return;

				m_connected = false;
				teardown();

				LIBNETWRK_INFO("%s disconnected", this->m_name.c_str());
			}

			/// <summary>
			/// Processes a single message if the queue is not empty.
			/// </summary>
			/// <returns>true if a message has been processed, false if it hasn't</returns>
			bool process_single_message() {
				try {
					if (this->m_incoming_messages.empty())
						return false;

					message_t msg = this->m_incoming_messages.pop_front().m_msg;
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

			/// <summary>
			/// Process messages while client is connected. This is a blocking function.
			/// </summary>
			void process_messages() {
				_process_messages();
			}

			/// <summary>
			/// Process messages while client is connected. 
			/// This function runs asynchronously until the client stops.
			/// </summary>
			void process_messages_async() {
				m_process_messages_thread = std::thread([&] { _process_messages(); });
			}

			/// <summary>
			/// Send a message
			/// </summary>
			/// <param name="message">: message to send</param>
			void send(message_t& message) {
				if (m_connection && m_connected) {
					if (m_connection->is_alive()) {
						m_connection->send(std::make_shared<message_t>(std::move(message)));
					}
					else {
						on_disconnect();
						disconnect();
					}
				}
			}

		protected:
			virtual void on_message(message_t& msg) {}

			virtual void on_disconnect() {}

			virtual bool _connect(const char* host, const unsigned short port) {
				return false;
			}

			void teardown() {
				if (this->m_context)
					if (!this->m_context->stopped())
						this->m_context->stop();

				if (m_connection)
					if (m_connection->is_alive())
						m_connection->stop();

				this->m_incoming_messages.cancel_wait();

				if (m_context_thread.joinable())
					m_context_thread.join();

				if (m_process_messages_thread.joinable())
					m_process_messages_thread.join();
			}

			void start_context() {
				m_context_thread = std::thread([this] { this->m_context->run(); });
			}

		private:
			void _process_messages(size_t max_messages = -1) {
				while (m_connected) {
					this->m_incoming_messages.wait();

					try {
						size_t message_count = 0;
						while (message_count < max_messages && !this->m_incoming_messages.empty()) {
							message_t msg = this->m_incoming_messages.pop_front().m_msg;
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