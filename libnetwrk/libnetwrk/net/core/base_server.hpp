#pragma once

#include "libnetwrk/net/core/base_connection.hpp"
#include "libnetwrk/net/core/context.hpp"
#include "libnetwrk/net/core/messages/owned_message.hpp"

#include <chrono>
#include <list>

namespace libnetwrk {
    template<typename Command, typename Serialize, typename Storage>
    class base_server : public context<Command, Serialize, Storage> {
    public:
        using base_server_t     = base_server<Command, Serialize, Storage>;
        using base_context_t    = context<Command, Serialize, Storage>;
        using message_t         = message<Command, Serialize>;
        using owned_message_t   = owned_message<Command, Serialize, Storage>;
        using base_connection_t = base_connection<Command, Serialize, Storage>;
        
        using guard_t = std::lock_guard<std::mutex>;
        using timer_t = asio::steady_timer;

        using send_predicate = std::function<bool(std::shared_ptr<base_connection_t>)>;

    public:
        base_server()                     = delete;
        base_server(const base_server_t&) = delete;
        base_server(base_server_t&&)      = default;

        base_server(const std::string& name = "base server") 
            : base_context_t(name, context_owner::server) {}

        virtual ~base_server() {
            m_running = false;
            teardown();
        };

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

            bool started = impl_start(host, port);

            if (started) {
                ev_service_started();
                m_running = true;
            }

            return started;
        }

        /// <summary>
        /// Stop server
        /// </summary>
        virtual void stop() = 0;

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

                ev_message(msg);
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
            impl_process_messages();
        }

        /// <summary>
        /// Process messages while server is running. 
        /// This function runs asynchronously until the server stops.
        /// </summary>
        void process_messages_async() {
            m_process_messages_thread = std::thread([&] { impl_process_messages(); });
        }

        /// <summary>
        /// Send a message to client.
        /// Message object after sending should be considered in an undefined state and
        /// shouldn't be used further without reassigning.
        /// </summary>
        /// <param name="client">: client to send to</param>
        /// <param name="message">: message to send</param>
        void send(std::shared_ptr<base_connection_t> client, message_t& message) {
            impl_send(client, std::make_shared<message_t>(std::move(message)));
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
                impl_send(client, msg);
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
                    impl_send(client, msg);
                }
            }
        }

    protected:
        bool     m_running = false;
        uint64_t m_ids     = 0U;

        std::list<std::shared_ptr<base_connection_t>> m_connections;
        std::mutex                                    m_connections_mutex;

    protected:
        // Called when the service was successfuly started
        virtual void ev_service_started() = 0;
        
        // Called when service stopped
        virtual void ev_service_stopped() = 0;
        
        // Called when processing messages
        virtual void ev_message(owned_message_t& msg) = 0;

        // Called before client is fully accepted
        // Allows performing checks on client before accepting (blacklist, whitelist)
        virtual bool ev_before_client_connected(std::shared_ptr<base_connection_t> client) = 0;

        // Called when a client has connected
        virtual void ev_client_connected(std::shared_ptr<base_connection_t> client) = 0;
        
        // Called when a client has disconnected
        virtual void ev_client_disconnected(std::shared_ptr<base_connection_t> client) = 0;

    protected:
        // Service start implementation
        virtual bool impl_start(const char* host, const unsigned short port) = 0;

        // Client accept implementation
        virtual void impl_accept() = 0;

    protected:
        void teardown() {
            if (this->m_gc_timer)
                this->m_gc_timer->cancel();

            if (this->asio_context && !this->asio_context->stopped())
                this->asio_context->stop();

            if (m_context_thread.joinable())
                m_context_thread.join();

            this->incoming_messages.cancel_wait();

            if (m_process_messages_thread.joinable())
                m_process_messages_thread.join();

            LIBNETWRK_INFO(this->name, "stopped");
        };

        void start_context() {
            m_gc_timer = std::make_unique<timer_t>(*this->asio_context, std::chrono::seconds(15));
            m_gc_timer->async_wait(std::bind(&base_server::impl_gc, this, std::placeholders::_1));
            
            m_context_thread = std::thread([this] { this->asio_context->run(); });
        }

    private:
        std::thread m_context_thread;
        std::thread m_process_messages_thread;

        std::unique_ptr<timer_t> m_gc_timer;

    private:
        void internal_ev_client_disconnected(std::shared_ptr<base_connection_t> client) override final {
            LIBNETWRK_INFO(this->name, "client disconnected");
            ev_client_disconnected(client);
        }

    private:
        void impl_send(std::shared_ptr<base_connection_t>& client, std::shared_ptr<message_t> message) {
            if (client && client->is_alive())
                client->send(message);
        }

        void impl_process_messages() {
            while (m_running) {
                this->incoming_messages.wait();

                while (!this->incoming_messages.empty()) {
                    owned_message_t msg = this->incoming_messages.pop_front();
                    ev_message(msg);
                }
            }
        }

        void impl_gc(const std::error_code& ec) {
            if (ec) {
                LIBNETWRK_ERROR(this->name, "failed to run gc | {}", ec.message());
                return;
            }

            std::unique_lock<std::mutex> guard(m_connections_mutex);

            size_t prev_size = m_connections.size();

            m_connections.remove_if([this](auto& client) {
                if (!client)
                    return true;

                if (!client->is_alive()) {
                    internal_ev_client_disconnected(client);
                    return true;
                }

                return false;
            });

            LIBNETWRK_INFO(this->name, "gc tc: {} rc: {}", m_connections.size(), prev_size - m_connections.size());

            m_gc_timer->expires_at(m_gc_timer->expiry() + std::chrono::seconds(15));
            m_gc_timer->async_wait(std::bind(&base_server::impl_gc, this, std::placeholders::_1));
        }
    };
}
