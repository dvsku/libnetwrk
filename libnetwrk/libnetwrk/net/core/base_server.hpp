#pragma once

#include "libnetwrk/net/core/base_connection.hpp"
#include "libnetwrk/net/core/context.hpp"
#include "libnetwrk/net/core/messages/owned_message.hpp"

#include <chrono>
#include <list>

namespace libnetwrk {
    template<typename Tcommand, typename Tserialize, typename Tstorage>
    class base_server : public context<Tcommand, Tserialize, Tstorage> {
    public:
        using base_server_t   = base_server<Tcommand, Tserialize, Tstorage>;
        using base_context_t  = context<Tcommand, Tserialize, Tstorage>;
        using message_t       = message<Tcommand, Tserialize>;
        using owned_message_t = owned_message<Tcommand, Tserialize, Tstorage>;
        using connection_t    = base_connection<Tcommand, Tserialize, Tstorage>;
        
        using guard_t = std::lock_guard<std::mutex>;

        using send_predicate = std::function<bool(std::shared_ptr<connection_t>)>;

    public:
        base_server()                     = delete;
        base_server(const base_server_t&) = delete;
        base_server(base_server_t&&)      = default;

        base_server(const std::string& name = "base server") 
            : base_context_t(name, context_owner::server) {}

        virtual ~base_server() {}

        base_server_t& operator=(const base_server&) = delete;
        base_server_t& operator=(base_server&&)      = default;

    public:
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

            if (started)
                m_gc_thread = std::thread([this] { _gc(); });

            return started;
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
                if (this->incoming_messages.empty())
                    return false;

                owned_message_t msg = this->incoming_messages.pop_front();

                on_message(msg);
            }
            catch (const std::exception& e) {
                LIBNETWRK_ERROR(this->name, "process_message() fail | {}", e.what());
                return false;
            }
            catch (...) {
                LIBNETWRK_ERROR(this->name, "process_message() fail | undefined reason");
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
        void send(std::shared_ptr<connection_t> client, message_t& message) {
            _send(client, std::make_shared<message_t>(std::move(message)));
        }

        /// <summary>
        /// Send a message to all clients.
        /// Message object after sending should be considered in an undefined state and
        /// shouldn't be used further without reassigning.
        /// </summary>
        /// <param name="message">: message to send</param>
        void send_all(message_t& message) {
            auto msg = std::make_shared<message_t>(std::move(message));

            guard_t guard(m_connections_mutex);
            for (auto& client : m_connections)
                _send(client, msg);
        }

        /// <summary>
        /// Send a message to clients that satisfy the predicate.
        /// Message object after sending should be considered in an undefined state and
        /// shouldn't be used further without reassigning.
        /// </summary>
        /// <param name="message">: message to send</param>
        /// <param name="predicate">: predicate for sending to client</param>
        void send_all(message_t& message, send_predicate predicate) {
            auto msg = std::make_shared<message_t>(std::move(message));

            guard_t guard(m_connections_mutex);
            for (auto& client : m_connections) {
                if (client && predicate(client)) {
                    _send(client, msg);
                }
            }
        }

        /// <summary>
        /// Stop server
        /// </summary>
        virtual void stop() {
            if (!m_running) return;

            m_running = false;

            if (this->context && !this->context->stopped())
                this->context->stop();

            if (m_context_thread.joinable())
                m_context_thread.join();

            this->incoming_messages.cancel_wait();

            if (m_process_messages_thread.joinable())
                m_process_messages_thread.join();

            m_gc_cv.notify_all();

            if (m_gc_thread.joinable())
                m_gc_thread.join();

            LIBNETWRK_INFO(this->name, "stopped");
        };

    protected:
        bool     m_running = false;
        uint64_t m_ids     = 0U;

        std::list<std::shared_ptr<connection_t>> m_connections;
        std::mutex                               m_connections_mutex;

    protected:
        virtual void on_message(owned_message_t& msg) {}

        virtual void on_client_disconnect(std::shared_ptr<connection_t> client) {
            LIBNETWRK_INFO(this->name, "client disconnected");
        }

        virtual bool _start(const char* host, const unsigned short port) = 0;

        virtual void _accept() = 0;

        void start_context() {
            m_context_thread = std::thread([this] { this->context->run(); });
        }

    private:
        std::thread m_context_thread;
        std::thread m_process_messages_thread;

        std::thread m_gc_thread;
        std::condition_variable m_gc_cv;

    private:
        void _send(std::shared_ptr<connection_t>& client, std::shared_ptr<message_t> message) {
            if (client && client->is_alive())
                client->send(message);
        }

        void _process_messages() {
            while (m_running) {
                this->incoming_messages.wait();

                while (!this->incoming_messages.empty()) {
                    owned_message_t msg = this->incoming_messages.pop_front();
                    on_message(msg);
                }
            }
        }

        void _gc() {
            while (m_running) {
                std::unique_lock<std::mutex> guard(m_connections_mutex);

                auto prev_size = m_connections.size();

                m_connections.remove_if([this](auto& client) {
                    if (!client) 
                        return true;

                    if (!client->is_alive()) {
                        on_client_disconnect(client);
                        return true;
                    }

                    return false;
                });

                if (prev_size - m_connections.size())
                    LIBNETWRK_INFO(this->name, "gc tc: {} rc: {}", m_connections.size(), prev_size - m_connections.size());

                m_gc_cv.wait_for(guard, std::chrono::seconds(15));
            }
        }
    };
}
