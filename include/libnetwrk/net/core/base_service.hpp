#pragma once

#include "libnetwrk/net/core/context.hpp"
#include "libnetwrk/net/core/base_client_connection.hpp"

#include <chrono>
#include <list>

namespace libnetwrk {
    template<typename Desc, typename Socket>
    class base_service : public context<Desc, base_client_connection<Desc, Socket>> {
    public:
        // This service type
        using service_t = base_service<Desc, Socket>;

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
        uint8_t gc_freq_sec = 15U;

    public:
        base_service()                 = delete;
        base_service(const service_t&) = delete;
        base_service(service_t&&)      = default;

        service_t& operator=(const service_t&) = delete;
        service_t& operator=(service_t&&)      = default;

        base_service(const std::string& name)
            : context_t(name) {}

    public:
        /*
            Get service status.
        */
        bool running() {
            return this->m_status == service_status::started;
        }

        /*
            Start service.
        */
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

        /*
            Stop service.
        */
        virtual void stop() = 0;

        /*
            Queue up a function to run.
        */
        void queue_async_job(std::function<void()> const& lambda) {
            asio::post(*(this->io_context), lambda);
        }

        /*
            Send a message to client.
            Message object after sending should be considered in an undefined state and
            shouldn't be used further without reassigning.
        */
        void send(std::shared_ptr<connection_t> client, message_t& message) {
            impl_send(client, std::make_shared<message_t>(std::move(message)));
        }

        /*
            Send a message to all clients.
            Message object after sending should be considered in an undefined state and
            shouldn't be used further without reassigning.
        */
        void send_all(message_t& message) {
            auto msg = std::make_shared<message_t>(std::move(message));

            std::lock_guard<std::mutex> guard(m_connections_mutex);
            for (auto& client : m_connections)
                impl_send(client, msg);
        }

        /*
            Send a message to clients that satisfy the predicate.
            Message object after sending should be considered in an undefined state and
            shouldn't be used further without reassigning.
        */
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
        virtual ~base_service() {
            if (this->m_status == service_status::stopped)
                return;

            teardown();
            this->m_status = service_status::stopped;
        };

    protected:
        /*
            Called when the service was successfully started.
        */
        virtual void ev_service_started() {};

        /*
            Called when service stopped.
        */
        virtual void ev_service_stopped() {};

        /*
            Called when processing messages.
        */
        virtual void ev_message(owned_message_t& msg) override {};

        /*
            Called before client is fully accepted.
            Allows performing checks on client before accepting (blacklist, whitelist).
        */
        virtual bool ev_before_client_connected(std::shared_ptr<connection_t> client) { return true; };

        /*
            Called when a client has connected.
        */
        virtual void ev_client_connected(std::shared_ptr<connection_t> client) {};
        
        /*
            Called when a client has disconnected.
        */
        virtual void ev_client_disconnected(std::shared_ptr<connection_t> client) {};

    protected:
        /*
            Service start implementation.
        */
        virtual bool impl_start(const char* host, const unsigned short port) = 0;

        /*
            Pre process message data before writing.
        */
        virtual void pre_process_message(message_t::buffer_t& buffer) override {}

        /*
            Post process message data after reading.
        */
        virtual void post_process_message(message_t::buffer_t& buffer) override {}

    protected:
        void teardown() {
            if (this->m_gc_timer)
                this->m_gc_timer->cancel();

            if (this->io_context && !this->io_context->stopped())
                this->io_context->stop();

            if (m_context_thread.joinable())
                m_context_thread.join();

            {
                std::lock_guard<std::mutex> guard(this->m_incoming_mutex);

                this->m_incoming_messages        = {};
                this->m_incoming_system_messages = {};
            }

            this->m_cv.notify_all();

            if (this->m_process_messages_thread.joinable())
                this->m_process_messages_thread.join();

            LIBNETWRK_INFO(this->name, "Stopped.");
        };

        void start_context() {
            m_gc_timer = std::make_unique<timer_t>(*this->io_context, std::chrono::seconds(gc_freq_sec));
            m_gc_timer->async_wait(std::bind(&base_service::impl_gc, this, std::placeholders::_1));
            
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
                    std::lock_guard<std::mutex> guard(this->m_incoming_mutex);

                    if (!this->m_incoming_system_messages.empty()) {
                        message = this->m_incoming_system_messages.front();
                        this->m_incoming_system_messages.pop();
                    }
                    else {
                        message = this->m_incoming_messages.front();
                        this->m_incoming_messages.pop();
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
                LIBNETWRK_ERROR(this->name, "Failed to process message. | Critical fail.");
                return false;
            }

            return true;
        }

        /*
            Client disconnected callback from connection.
        */
        void internal_ev_client_disconnected(std::shared_ptr<connection_t> client) override final {
            LIBNETWRK_INFO(this->name, "Client disconnected.");
        }

        void ev_system_message(owned_message_t& msg) override final {
            system_command command = static_cast<system_command>(msg.msg.command());

            switch (command) {
                case system_command::c2s_verify: return on_system_verify_message(msg);
                default:                         return;
            }
        }

        void on_system_verify_message(owned_message_t& msg) {
            LIBNETWRK_DEBUG(this->name, "Received verify response.");

            auth::answer_t answer{};    
            msg.msg >> answer;

            if (!auth::is_correct(msg.sender->auth_question, answer))
                return msg.sender->stop();

            msg.sender->is_authenticated.store(true);

            message_t response{};
            response.head.type    = message_type::system;
            response.head.command = static_cast<uint64_t>(system_command::s2c_verify_ok);

            msg.sender->send(response);
        }

    private:
        void impl_send(std::shared_ptr<connection_t>& client, std::shared_ptr<message_t> message) {
            if (client && client->is_connected())
                client->send(message);
        }

        void impl_gc(const std::error_code& ec) {
            if (ec) {
                if (ec != asio::error::operation_aborted)
                    LIBNETWRK_ERROR(this->name, "Failed to run GC. | {}", ec.message());

                return;
            }

            std::unique_lock<std::mutex> guard(m_connections_mutex);

            size_t prev_size = m_connections.size();

            m_connections.remove_if([this](auto& client) {
                if (!client)
                    return true;

                if (!client->is_connected()) {
                    ev_client_disconnected(client);
                    return true;
                }

                if (!client->is_authenticated.load()) {
                    uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();

                    if (timestamp > client->auth_timeout_timestamp) {
                        client->stop();
                        LIBNETWRK_VERBOSE(this->name, "Auth timeout. Client disconnected.");
                        ev_client_disconnected(client);
                        return true;
                    }
                }

                return false;
            });

            LIBNETWRK_VERBOSE(this->name, "GC tc: {} rc: {}", m_connections.size(), prev_size - m_connections.size());

            m_gc_timer->expires_at(m_gc_timer->expiry() + std::chrono::seconds(gc_freq_sec));
            m_gc_timer->async_wait(std::bind(&base_service::impl_gc, this, std::placeholders::_1));
        }
    };
}
