#pragma once

#include "libnetwrk/net/core/base_connection.hpp"
#include "libnetwrk/net/core/context.hpp"
#include "libnetwrk/net/core/messages/owned_message.hpp"

namespace libnetwrk {
    template<typename Desc>
    class base_client : public context<Desc> {
    public:
        using base_client_t     = base_client<Desc>;
        using base_context_t    = context<Desc>;
        using message_t         = message<Desc>;
        using owned_message_t   = owned_message<Desc>;
        using base_connection_t = base_connection<Desc>;

    public:
        base_client()                   = delete;
        base_client(const base_client&) = delete;
        base_client(base_client&&)      = default;
    
        base_client(const std::string& name = "base client") 
            : base_context_t(name, context_owner::client) {}
    
        virtual ~base_client() {
            if (this->m_status == service_status::stopped || this->m_status == service_status::stopping)
                return;

            this->m_status = service_status::stopping;
            teardown();
            this->m_status = service_status::stopped;
        };

        base_client_t& operator=(const base_client_t&) = delete;
        base_client_t& operator=(base_client_t&&)      = default;

    public:
        /// <summary>
        /// Client status
        /// </summary>
        /// <returns>true if connected, false if disconnected</returns>
        bool connected() {
            return this->m_status == service_status::started;
        }
    
        /// <summary>
        /// Connect to TCP server
        /// </summary>
        /// <param name="host">: IPv4 address</param>
        /// <param name="port">: port</param>
        /// <returns>true if connected, false if not</returns>
        bool connect(const char* host, const unsigned short port) {
            if (this->m_status != service_status::stopped) 
                return false;

            this->m_status = service_status::starting;

            bool connected = impl_connect(host, port);

            if (connected) {
                ev_connected();
                this->m_status = service_status::started;
            }
            else {
                this->m_status = service_status::stopped;
            }
            
            return connected;
        }
    
        /// <summary>
        /// Disconnect the client and clean up
        /// </summary>
        void disconnect() {
            if (this->m_status != service_status::started) 
                return;

            this->m_status = service_status::stopping;

            teardown();
            ev_disconnected();

            this->m_status = service_status::stopped;
        }
    
        /// <summary>
        /// Send a message
        /// </summary>
        /// <param name="message">: message to send</param>
        void send(message_t& message) {
            if (m_connection && connected()) {
                if (m_connection->is_alive()) {
                    m_connection->send(std::make_shared<message_t>(std::move(message)));
                }
                else {
                    disconnect();
                }
            }
        }

    protected:
        std::shared_ptr<base_connection_t> m_connection;
    
    protected:
        // Called when successfully connected
        virtual void ev_connected() {};

        // Called when disconnected
        virtual void ev_disconnected() {};

    protected:
        // Connect implementation
        virtual bool impl_connect(const char* host, const unsigned short port) = 0;

    protected:
        void teardown() {
            if (this->asio_context && !this->asio_context->stopped())
                this->asio_context->stop();
    
            if (m_connection && m_connection->is_alive())
                m_connection->stop();
    
            m_connection.reset();
    
            if (m_context_thread.joinable())
                m_context_thread.join();

            {
                std::lock_guard<std::mutex> guard(this->incoming_mutex);
                this->incoming_messages = {};
            }
    
            if (this->m_process_messages_thread.joinable())
                this->m_process_messages_thread.join();

            LIBNETWRK_INFO(this->name, "disconnected");
        }
    
        void start_context() {
            m_context_thread = std::thread([this] { this->asio_context->run(); });
        }

    private:
        std::thread m_context_thread;

    private:
        void internal_ev_client_disconnected(std::shared_ptr<base_connection_t> client) override final {
            std::thread thread = std::thread([this] {
                this->disconnect();
            });
            thread.detach();
        }
    };
}
