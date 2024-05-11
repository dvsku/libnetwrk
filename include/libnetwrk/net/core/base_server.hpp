#pragma once

#include "libnetwrk/net/core/context.hpp"
#include "libnetwrk/net/core/base_client_connection.hpp"

#include <chrono>
#include <list>

namespace libnetwrk {
    template<typename Desc, typename Socket>
    class base_server : public context<Desc, base_client_connection<Desc, Socket>> {
    public:
        // This service type
        using service_t = base_server<Desc, Socket>;

        // Connection type for this service
        using connection_t = base_client_connection<Desc, Socket>;

        // Context type for this service
        using context_t = context<Desc, connection_t>;

        // Message type
        using message_t = connection_t::message_t;

        // Owned message type for this service
        using owned_message_t = connection_t::owned_message_t;

        // Send predicate
        using send_predicate = std::function<bool(std::shared_ptr<connection_t>)>;

    private:
        // Timer type
        using timer_t = asio::steady_timer;

    public:
        base_server()                 = delete;
        base_server(const service_t&) = delete;
        base_server(service_t&&)      = default;

        service_t& operator=(const service_t&) = delete;
        service_t& operator=(service_t&&)      = default;

        base_server(const std::string& name)
            : context_t(name) {}

        virtual ~base_server() {
            if (this->m_status == service_status::stopped)
                return;

            teardown();
            this->m_status = service_status::stopped;
        };

    public:
        /// <summary>
        /// Get server status
        /// </summary>
        /// <returns>true if running, false if stopped</returns>
        bool running() {
            return this->m_status == service_status::started;
        }

        /// <summary>
        /// Start server
        /// </summary>
        /// <param name="host">: IPv4 address</param>
        /// <param name="port">: port</param>
        /// <returns>true if started, false if failed to start</returns>
        bool start(const char* host, const unsigned short port) {
            if (this->m_status != service_status::stopped)
                return false;

            this->m_status = service_status::starting;

            bool started = impl_start(host, port);

            if (started) {
                ev_service_started();
                this->m_status = service_status::started;
            }
            else {
                this->m_status = service_status::stopped;
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
            asio::post(*(this->io_context), lambda);
        }

        /// <summary>
        /// Send a message to client.
        /// Message object after sending should be considered in an undefined state and
        /// shouldn't be used further without reassigning.
        /// </summary>
        /// <param name="client">: client to send to</param>
        /// <param name="message">: message to send</param>
        void send(std::shared_ptr<connection_t> client, message_t& message) {
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

            std::lock_guard<std::mutex> guard(m_connections_mutex);
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

            std::lock_guard<std::mutex> guard(m_connections_mutex);
            for (auto& client : m_connections) {
                if (client && predicate(client)) {
                    impl_send(client, msg);
                }
            }
        }

    protected:
        uint64_t m_ids     = 0U;

        std::list<std::shared_ptr<connection_t>> m_connections;
        std::mutex                               m_connections_mutex;

    protected:
        // Called when the service was successfuly started
        virtual void ev_service_started() {};
        
        // Called when service stopped
        virtual void ev_service_stopped() {};

        // Called when processing messages
        virtual void ev_message(owned_message_t& msg) override {};

        // Called before client is fully accepted
        // Allows performing checks on client before accepting (blacklist, whitelist)
        virtual bool ev_before_client_connected(std::shared_ptr<connection_t> client) { return true; };

        // Called when a client has connected
        virtual void ev_client_connected(std::shared_ptr<connection_t> client) {};
        
        // Called when a client has disconnected
        virtual void ev_client_disconnected(std::shared_ptr<connection_t> client) {};

    protected:
        // Service start implementation
        virtual bool impl_start(const char* host, const unsigned short port) = 0;

        // Client accept implementation
        virtual void impl_accept() = 0;

    protected:
        void teardown() {
            if (this->m_gc_timer)
                this->m_gc_timer->cancel();

            if (this->io_context && !this->io_context->stopped())
                this->io_context->stop();

            if (m_context_thread.joinable())
                m_context_thread.join();

            {
                std::lock_guard<std::mutex> guard(this->incoming_mutex);

                this->incoming_messages        = {};
                this->incoming_system_messages = {};
            }

            if (this->m_process_messages_thread.joinable())
                this->m_process_messages_thread.join();

            LIBNETWRK_INFO(this->name, "stopped");
        };

        void start_context() {
            m_gc_timer = std::make_unique<timer_t>(*this->io_context, std::chrono::seconds(15));
            m_gc_timer->async_wait(std::bind(&base_server::impl_gc, this, std::placeholders::_1));
            
            m_context_thread = std::thread([this] { this->io_context->run(); });
        }

    private:
        std::thread              m_context_thread;
        std::unique_ptr<timer_t> m_gc_timer;

    private:
        bool internal_process_message() override final {
            try {
                owned_message_t message;

                {
                    std::lock_guard<std::mutex> guard(this->incoming_mutex);

                    if (this->incoming_system_messages.empty() && this->incoming_messages.empty())
                        return false;

                    if (!this->incoming_system_messages.empty()) {
                        message = this->incoming_system_messages.front();
                        this->incoming_system_messages.pop();
                    }
                    else {
                        message = this->incoming_messages.front();
                        this->incoming_messages.pop();
                    }
                }

                if (message.msg.head.type == message_type::system) {
                    ev_system_message(message);
                }
                else {
                    ev_message(message);
                }
            }
            catch (const std::exception& e) {
                (void)e;

                LIBNETWRK_ERROR(this->name, "Failed to process message. | {}", e.what());
                return false;
            }
            catch (...) {
                LIBNETWRK_ERROR(this->name, "Failed to process message. | Critical fail");
                return false;
            }

            return true;
        }

        void internal_ev_client_disconnected(std::shared_ptr<connection_t> client) override final {
            LIBNETWRK_INFO(this->name, "client disconnected");
            ev_client_disconnected(client);
        }

        void ev_system_message(owned_message_t& msg) override final {
            system_command command = static_cast<system_command>(msg.msg.command());
        }

    private:
        void impl_send(std::shared_ptr<connection_t>& client, std::shared_ptr<message_t> message) {
            if (client && client->is_connected())
                client->send(message);
        }

        void impl_gc(const std::error_code& ec) {
            if (ec) {
                if (ec != asio::error::operation_aborted)
                    LIBNETWRK_ERROR(this->name, "failed to run gc | {}", ec.message());

                return;
            }

            std::unique_lock<std::mutex> guard(m_connections_mutex);

            size_t prev_size = m_connections.size();

            m_connections.remove_if([this](auto& client) {
                if (!client)
                    return true;

                if (!client->is_connected()) {
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
