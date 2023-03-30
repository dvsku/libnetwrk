#ifndef LIBNETWRK_NET_COMMON_BASE_SERVER_HPP
#define LIBNETWRK_NET_COMMON_BASE_SERVER_HPP

#include <type_traits>

#include "libnetwrk/net/definitions.hpp"
#include "libnetwrk/net/macros.hpp"
#include "libnetwrk/net/message.hpp"
#include "libnetwrk/net/common/containers/tsdeque.hpp"
#include "libnetwrk/net/common/base_connection.hpp"
#include "libnetwrk/net/common/serialization/serializers/binary_serializer.hpp"

namespace libnetwrk::net::common {
	template <typename command_type,
		typename serializer = libnetwrk::net::common::binary_serializer,
		typename storage = libnetwrk::nothing>
	class base_server {
		public:
			typedef libnetwrk::net::message<command_type, serializer> message_t;
			typedef std::shared_ptr<message_t> message_t_ptr;
			typedef libnetwrk::net::owned_message<command_type, serializer, storage> owned_message_t;

			typedef base_connection<command_type, serializer, storage> base_connection_t;
			typedef std::shared_ptr<base_connection_t> base_connection_t_ptr;

			// function with signature: bool f(const storage&)
			typedef std::function<bool(const storage&)> send_condition;

		protected:
			std::string m_name;
			bool m_running = false;

			context_ptr m_context;
			libnetwrk::net::common::tsdeque<owned_message_t> m_incoming_messages;

			std::deque<base_connection_t_ptr> m_connections;

			std::thread m_context_thread;
			std::thread m_process_messages_thread;

		public:
			base_server(const std::string& name = "base server") {
				LIBNETWRK_STATIC_ASSERT_OR_THROW(std::is_enum<command_type>::value,
					"server command_type template arg can only be an enum");

				m_name = name;
			}

			/// <summary>
			/// Get ASIO io_context
			/// </summary>
			/// <returns>pointer to ASIO io_context</returns>
			context_ptr context() {
				return this->m_context;
			}

			/// <summary>
			/// Get server status
			/// </summary>
			/// <returns>true if running, false if stopped</returns>
			bool running() {
				return m_running;
			}

			/// <summary>
			/// Start server
			/// </summary>
			/// <param name="host">: IPv4 address</param>
			/// <param name="port">: port</param>
			/// <returns>true if started, false if failed to start</returns>
			bool start(const char* host, const unsigned short port) {
				if (m_running) return false;
				return _start(host, port);
			}

			/// <summary>
			/// Start server
			/// </summary>
			/// <param name="ep">: endpoint -> localhost (loopback) or v4 (all IPv4 adaptors)</param>
			/// <param name="port">: port</param>
			/// <returns>true if started, false if failed to start</returns>
			bool start(libnetwrk::endpoint ep, const unsigned short port) {
				switch (ep) {
					case libnetwrk::endpoint::localhost:	return start("127.0.0.1", port);
					case libnetwrk::endpoint::v4:			return start("0.0.0.0", port);
					default:								return false;
				}
			}

			/// <summary>
			/// Queue up a function to run
			/// </summary>
			/// <param name="lambda">: function to run</param>
			void queue_async_job(std::function<void()> const& lambda) {
				asio::post(*(this->m_context), lambda);
			}

			/// <summary>
			/// Processes a single message if the queue is not empty.
			/// </summary>
			/// <returns>true if a message has been processed, false if it hasn't</returns>
			bool process_message() {
				try {
					if (m_incoming_messages.empty())
						return false;

					owned_message_t msg =
						m_incoming_messages.pop_front();

					on_message(msg);
				}
				catch (const std::exception& e) {
					LIBNETWRK_ERROR("process_message() fail | %s", e.what());
					return false;
				}
				catch (...) {
					LIBNETWRK_ERROR("process_message() fail | undefined reason");
					return false;
				}

				return true;
			}

			/// <summary>
			/// Process messages while server is running. This is a blocking function.
			/// </summary>
			void process_messages() {
				_process_messages();
			}

			/// <summary>
			/// Process messages while server is running. 
			/// This function runs asynchronously until the server stops.
			/// </summary>
			void process_messages_async() {
				m_process_messages_thread = std::thread([&] { _process_messages(); });
			}

			/// <summary>
			/// Send a message to client.
			/// Message object after sending should be considered in an undefined state and
			/// shouldn't be used further without reassigning.
			/// </summary>
			/// <param name="client">: client to send to</param>
			/// <param name="message">: message to send</param>
			void send(base_connection_t_ptr& client, message_t& message) {
				_send(client, std::make_shared<message_t>(std::move(message)));
			}

			/// <summary>
			/// Send a message to all clients.
			/// Message object after sending should be considered in an undefined state and
			/// shouldn't be used further without reassigning.
			/// </summary>
			/// <param name="message">: message to send</param>
			void send_all(message_t& message) {
				message_t_ptr ptr = std::make_shared<message_t>(std::move(message));
				
				for (auto& client : m_connections)
					_send(client, ptr);
			}

			/// <summary>
			/// Send a message to clients that satisfy the condition.
			/// Message object after sending should be considered in an undefined state and
			/// shouldn't be used further without reassigning.
			/// </summary>
			/// <param name="message">: message to send</param>
			/// <param name="condition">: condition for sending to client</param>
			void send_all(message_t& message, send_condition condition) {
				message_t_ptr ptr = std::make_shared<message_t>(std::move(message));

				for (auto& client : m_connections) {
					if (client)
						if (condition(client->get_storage()))
							_send(client, ptr);
				}
			}

			/// <summary>
			/// Stop server
			/// </summary>
			virtual void stop() {
				m_running = false;

				if (m_context)
					if (!m_context->stopped())
						m_context->stop();

				if (m_context_thread.joinable())
					m_context_thread.join();

				m_incoming_messages.cancel_wait();

				if (m_process_messages_thread.joinable())
					m_process_messages_thread.join();

				LIBNETWRK_INFO("%s stopped", m_name.c_str());
			};

		protected:
			virtual void on_message(owned_message_t& msg) {
				return;
			}

			virtual bool on_client_connect(base_connection_t_ptr client) {
				return true;
			}

			virtual void on_client_disconnect(base_connection_t_ptr client) {
				return;
			}

			virtual bool _start(const char* host, const unsigned short port) = 0;

			virtual void _accept() = 0;

		private:
			void _send(base_connection_t_ptr& client, const message_t_ptr& message) {
				if (client && client->is_alive()) {
					client->send(message);
				}
				else {
					on_client_disconnect(client);
					client.reset();
				}
			}

			void _process_messages(size_t max_messages = -1) {
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