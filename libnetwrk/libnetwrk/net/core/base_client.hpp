#pragma once

#include "libnetwrk/net/core/base_connection.hpp"
#include "libnetwrk/net/core/context.hpp"
#include "libnetwrk/net/core/messages/owned_message.hpp"

namespace libnetwrk {
    template<typename Tcommand, typename Tserialize, typename Tstorage>
    class base_client : public context<Tcommand, Tserialize, Tstorage> {
    public:
        using base_client_t   = base_client<Tcommand, Tserialize, Tstorage>;
        using base_context_t  = context<Tcommand, Tserialize, Tstorage>;
        using message_t       = message<Tcommand, Tserialize>;
        using owned_message_t = owned_message<Tcommand, Tserialize, Tstorage>;
        using connection_t    = base_connection<Tcommand, Tserialize, Tstorage>;

    public:
        base_client()                   = delete;
        base_client(const base_client&) = delete;
        base_client(base_client&&)      = default;
    
        base_client(const std::string& name = "base client") 
            : base_context_t(name, context_owner::client) {}
    
        virtual ~base_client() {
            disconnect();
        }

        base_client_t& operator=(const base_client_t&) = delete;
        base_client_t& operator=(base_client_t&&)      = default;

    public:
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
    
            on_disconnect();
            teardown();
    
            LIBNETWRK_INFO(this->name, "disconnected");
        }
    
        /// <summary>
        /// Processes a single message if the queue is not empty.
        /// </summary>
        /// <returns>true if a message has been processed, false if it hasn't</returns>
        bool process_message() {
            try {
                if (!m_connected || this->incoming_messages.empty())
                    return false;
    
                message_t msg = this->incoming_messages.pop_front().msg;
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
                    disconnect();
                }
            }
        }

    protected:
        bool                          m_connected = false;
        std::shared_ptr<connection_t> m_connection;
    
    protected:
        virtual void on_message(message_t& msg) {}
    
        virtual void on_connect()    {}
        virtual void on_disconnect() {}
    
        virtual bool _connect(const char* host, const unsigned short port) {
            return false;
        }

    protected:
        void teardown() {
            if (this->context)
                if (!this->context->stopped())
                    this->context->stop();
    
            if (m_connection)
                if (m_connection->is_alive())
                    m_connection->stop();
    
            m_connection.reset();
    
            this->incoming_messages.cancel_wait();
            this->incoming_messages.clear();
    
            if (m_context_thread.joinable())
                m_context_thread.join();
    
            if (m_process_messages_thread.joinable())
                m_process_messages_thread.join();
        }
    
        void start_context() {
            m_context_thread = std::thread([this] { this->context->run(); });
        }

    private:
        std::thread m_context_thread;
        std::thread m_process_messages_thread;

    private:
        void _process_messages() {
            while (m_connected) {
                this->incoming_messages.wait();
    
                try {
                    while (!this->incoming_messages.empty()) {
                        message_t msg = this->incoming_messages.pop_front().msg;
                        on_message(msg);
                    }
                }
                catch (const std::exception& e) {
                    LIBNETWRK_ERROR(this->name, "_process_messages() fail | {}", e.what());
                }
                catch (...) {
                    LIBNETWRK_ERROR(this->name, "_process_messages() fail | undefined reason");
                }
            }
        }
    };
}
