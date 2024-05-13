#pragma once

#include "libnetwrk/net/core/context.hpp"
#include "libnetwrk/net/core/base_service_connection.hpp"

namespace libnetwrk {
    template<typename Desc, typename Socket>
    class base_client : public context<Desc, base_service_connection<Desc, Socket>> {
    public:
        // This client type
        using client_t = base_client<Desc, Socket>;

        // Connection type for this client
        using connection_t = base_service_connection<Desc, Socket>;

        // Context type for this client
        using context_t = context<Desc, connection_t>;

        // Message type
        using message_t = connection_t::message_t;

        // Owned message type for this client
        using owned_message_t = connection_t::owned_message_t;

    public:
        base_client()                = delete;
        base_client(const client_t&) = delete;
        base_client(client_t&&)      = default;

        client_t& operator=(const client_t&) = delete;
        client_t& operator=(client_t&&)      = default;
    
        base_client(const std::string& name)
            : context_t(name) {}
    
        virtual ~base_client() {
            if (this->m_status == service_status::stopped || this->m_status == service_status::stopping)
                return;

            this->m_status = service_status::stopping;
            teardown();
            this->m_status = service_status::stopped;
        };

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
    
        /*
            Disconnect the client and clean up
        */
        virtual void disconnect() {}
    
        /// <summary>
        /// Send a message
        /// </summary>
        /// <param name="message">: message to send</param>
        void send(message_t& message) {
            if (m_connection && connected()) {
                if (m_connection->is_connected()) {
                    m_connection->send(std::make_shared<message_t>(std::move(message)));
                }
                else {
                    disconnect();
                }
            }
        }

    protected:
        std::shared_ptr<connection_t> m_connection;
    
    protected:
        // Called when successfully connected
        virtual void ev_connected() {};

        // Called when disconnected
        virtual void ev_disconnected() {};

        // Called when processing messages
        virtual void ev_message(owned_message_t& msg) override {};

    protected:
        // Connect implementation
        virtual bool impl_connect(const char* host, const unsigned short port) = 0;

        /*
            Pre process message data before writing
        */
        virtual void pre_process_message(message_t::buffer_t& buffer) override {}

        /*
            Post process message data after reading
        */
        virtual void post_process_message(message_t::buffer_t& buffer) override {}

    protected:
        void teardown() {
            if (this->io_context && !this->io_context->stopped())
                this->io_context->stop();
    
            if (m_connection && m_connection->is_connected())
                m_connection->stop();

            if (m_context_thread.joinable())
                m_context_thread.join();

            m_connection.reset();

            {
                std::lock_guard<std::mutex> guard(this->incoming_mutex);

                this->incoming_messages        = {};
                this->incoming_system_messages = {};
            }
    
            if (this->m_process_messages_thread.joinable())
                this->m_process_messages_thread.join();

            LIBNETWRK_INFO(this->name, "Disconnected.");
        }
    
        void start_context() {
            m_context_thread = std::thread([this] { this->io_context->run(); });
        }

    private:
        std::thread m_context_thread;

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
                LIBNETWRK_ERROR(this->name, "Failed to process message. | Critical fail.");
                return false;
            }

            return true;
        }

        void internal_ev_client_disconnected(std::shared_ptr<connection_t> client) override final {
            std::thread thread = std::thread([this] {
                this->disconnect();
            });
            thread.detach();
        }

        void ev_system_message(owned_message_t& msg) override final {
            system_command command = static_cast<system_command>(msg.msg.command());

            switch (command) {
                case system_command::s2c_verify:    return on_system_verify_message(msg);
                case system_command::s2c_verify_ok: return on_system_verify_ok_message(msg);
                default:                            return;
            }
        }

        void on_system_verify_message(owned_message_t& msg) {
            auth::question_t question{};
            auth::answer_t   answer{};

            msg.msg >> question;
            answer = auth::generate_auth_answer(question);

            message_t response;
            response.head.type    = message_type::system;
            response.head.command = static_cast<uint64_t>(system_command::c2s_verify);
            response << answer;
            
            send(response);
        }

        void on_system_verify_ok_message(owned_message_t& msg) {
            m_connection->is_authenticated.store(true);
        }
    };
}
