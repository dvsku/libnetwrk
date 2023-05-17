#ifndef LIBNETWRK_NET_COMMON_BASE_SERVER_HPP
#define LIBNETWRK_NET_COMMON_BASE_SERVER_HPP

#include <type_traits>
#include <list>
#include <chrono>

#include "libnetwrk/net/definitions.hpp"
#include "libnetwrk/net/macros.hpp"
#include "libnetwrk/net/message.hpp"
#include "libnetwrk/net/common/base_context.hpp"
#include "libnetwrk/net/common/containers/tsdeque.hpp"
#include "libnetwrk/net/common/base_connection.hpp"
#include "libnetwrk/net/common/serialization/serializers/binary_serializer.hpp"

namespace libnetwrk::net::common {
	template <typename command_type,
		typename serializer = libnetwrk::net::common::binary_serializer,
		typename storage = libnetwrk::nothing>
	class base_server : public base_context<command_type, serializer, storage> {
		public:
			typedef libnetwrk::net::message<command_type, serializer>					message_t;
			typedef std::shared_ptr<message_t>											message_t_ptr;
			typedef libnetwrk::net::owned_message<command_type, serializer, storage>	owned_message_t;

			typedef base_context<command_type, serializer, storage>			base_context_t;
			typedef base_connection<command_type, serializer, storage>		base_connection_t;
			typedef std::shared_ptr<base_connection_t>						base_connection_t_ptr;
			typedef base_connection_t_ptr									client_ptr;

			// function with signature: bool f(const client_ptr&)
			typedef std::function<bool(const client_ptr&)> send_condition;

		protected:
			bool m_running = false;
			uint64_t m_id_counter = 0U;

			std::list<base_connection_t_ptr> m_connections;
			std::mutex m_connections_mutex;

		private:
			std::thread m_context_thread;
			std::thread m_process_messages_thread;

			std::thread m_gc_thread;
			std::condition_variable m_gc_cv;

		public:
			base_server() = delete;
			base_server(const base_server&) = delete;
			base_server(base_server&&) = default;

			base_server(const std::string& name = "base server") : base_context_t(name, connection_owner::server) {
				LIBNETWRK_STATIC_ASSERT_OR_THROW(std::is_enum<command_type>::value,
					"server command_type template arg can only be an enum");
			}

			base_server& operator= (const base_server&) = delete;
			base_server& operator= (base_server&&) = default;

			virtual ~base_server() {}

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
				bool started = _start(host, port);

				if(started)
					m_gc_thread = std::thread([this] { _gc(); });

				return started;
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
					if (base_context_t::m_incoming_messages.empty())
						return false;

					owned_message_t msg =
						base_context_t::m_incoming_messages.pop_front();

					on_message(msg);
				}
				catch (const std::exception& e) {
					LIBNETWRK_ERROR_A(this->name(), "process_message() fail | %s", e.what());
					return false;
				}
				catch (...) {
					LIBNETWRK_ERROR_A(this->name(), "process_message() fail | undefined reason");
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
			void send(client_ptr& client, message_t& message) {
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
				
				libnetwrk_guard guard(m_connections_mutex);
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

				libnetwrk_guard guard(m_connections_mutex);
				for (auto& client : m_connections) {
					if (client)
						if (condition(client))
							_send(client, ptr);
				}
			}

			/// <summary>
			/// Stop server
			/// </summary>
			virtual void stop() {
				if (!m_running) return;

				m_running = false;

				if (base_context_t::m_context)
					if (!base_context_t::m_context->stopped())
						base_context_t::m_context->stop();

				if (m_context_thread.joinable())
					m_context_thread.join();

				base_context_t::m_incoming_messages.cancel_wait();

				if (m_process_messages_thread.joinable())
					m_process_messages_thread.join();

				m_gc_cv.notify_all();

				if (m_gc_thread.joinable())
					m_gc_thread.join();

				LIBNETWRK_INFO_A(this->name(), "stopped", base_context_t::m_name.c_str());
			};

		protected:
			virtual void on_message(owned_message_t& msg) {}

			virtual void on_client_disconnect(client_ptr client) {
				LIBNETWRK_INFO_A(this->name(), "client disconnected");
			}

			virtual bool _start(const char* host, const unsigned short port) = 0;

			virtual void _accept() = 0;

			void start_context() {
				m_context_thread = std::thread([this] { this->m_context->run(); });
			}

		private:
			void _send(client_ptr& client, const message_t_ptr& message) {
				if (client && client->is_alive())
					client->send(message);
			}

			void _process_messages(size_t max_messages = -1) {
				while (m_running) {
					base_context_t::m_incoming_messages.wait();
					size_t message_count = 0;
					while (message_count < max_messages && !base_context_t::m_incoming_messages.empty()) {
						owned_message_t msg = base_context_t::m_incoming_messages.pop_front();
						on_message(msg);
						message_count++;
					}
				}
			}

			void _gc() {
				while (m_running) {
					std::unique_lock<std::mutex> guard(m_connections_mutex);
					
					auto count = m_connections.remove_if([this](auto& client) {
						if (client == nullptr) return true;

						if (!client->is_alive()) {
							on_client_disconnect(client);
							return true;
						}
						
						return false;
					});

					if (count)
						LIBNETWRK_INFO_A(this->name(), "gc tc: %d rc: %d", m_connections.size(), count);

					m_gc_cv.wait_for(guard, std::chrono::seconds(15));
				}
			}
	};
}

#endif