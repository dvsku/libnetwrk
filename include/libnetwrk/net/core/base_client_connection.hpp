#pragma once

#include "libnetwrk/net/core/base_connection.hpp"
#include "libnetwrk/net/core/auth.hpp"
#include "libnetwrk/net/core/system_commands.hpp"

#include <memory>

namespace libnetwrk {
    template<typename Desc, typename Socket>
    class base_service;

    /*
        Base connection that represents a service's client connection
    */
    template<typename Desc, typename Socket>
    class base_client_connection 
        : public base_connection<Desc, Socket>, public std::enable_shared_from_this<base_client_connection<Desc, Socket>>
    {
    public:
        // This connection type
        using connection_t = base_client_connection<Desc, Socket>;

        // Socket type
        using socket_t = Socket;

        // Message type
        using message_t = message<Desc>;

        // Owned message type for this connection
        using owned_message_t = owned_message<Desc, connection_t>;

        // Context type for this connection
        using context_t = context<Desc, connection_t>;

        // Storage type for this connection
        using storage_t = typename Desc::storage_t;

        friend base_service<Desc, Socket>;
        
    public:
        auth::question_t auth_question{};
        uint64_t         auth_timeout_timestamp = 0U;

    public:
        base_client_connection()                              = delete;
        base_client_connection(const base_client_connection&) = delete;
        base_client_connection(base_client_connection&&)      = default;

        base_client_connection(context_t& context, socket_t socket)
            : base_connection<Desc, Socket>(std::move(socket)), m_context(context) {}

        base_client_connection& operator=(const base_client_connection&) = delete;
        base_client_connection& operator=(base_client_connection&&)      = default;

    public:
        /*
            Get connection storage.
        */
        storage_t& get_storage() {
            return m_storage;
        }

    public:
        /*
            Start read/write operations.
        */
        void start() override final {
            auth_question = auth::generate_auth_question();

            message_t request;
            request.head.type    = message_type::system;
            request.head.command = static_cast<uint64_t>(system_command::s2c_verify);
            request << auth_question;

            this->send(request);

            auto limit             = std::chrono::system_clock::now() + std::chrono::seconds(10);
            auth_timeout_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(limit.time_since_epoch()).count();

            asio::co_spawn(*m_context.io_context, co_read(), asio::detached);
        }

    protected:
        context_t& m_context;
        storage_t  m_storage;

    private:
        /*
            Called when there's a disconnect during read/write.
        */
        void internal_disconnect() override final {
            this->stop();
            this->m_context.internal_ev_client_disconnected(this->shared_from_this());
        }

        /*
            Called when there's a failure during read/write.
        */
        void internal_failure(std::error_code ec) override final {
            internal_disconnect();
            LIBNETWRK_ERROR(this->m_context.name, "Failed during read/write. | {}", ec.message());
        }

        asio::awaitable<void> co_read() {
            owned_message_t owned_message;
            std::error_code ec;

            while (true) {
                if (!this->is_connected())
                    break;

                co_await this->co_read_message(owned_message.msg, ec);

                if (ec) {
                    if (ec != asio::error::eof && ec != asio::error::connection_reset) {
                        LIBNETWRK_ERROR(this->m_context.name, "Failed during read/write. | {}", ec.message());
                    }

                    this->stop();
                    this->m_context.internal_ev_client_disconnected(this->shared_from_this());
                    break;
                }

                /*
                    If client is not authenticated, discard user messages
                */

                if (!this->is_authenticated.load() && owned_message.msg.head.type != message_type::system)
                    continue;

                owned_message.sender = this->shared_from_this();

                // Post process message data
                m_context.post_process_message(owned_message.msg.data);

                // Set new data size
                owned_message.msg.head.data_size = owned_message.msg.data.size();

                {
                    std::lock_guard<std::mutex> guard(this->m_context.m_incoming_mutex);

                    {
                        std::lock_guard<std::mutex> cv_lock(this->m_context.m_cv_mutex);

                        if (owned_message.msg.head.type == message_type::system) {
                            this->m_context.m_incoming_system_messages.push(std::move(owned_message));
                        }
                        else {
                            this->m_context.m_incoming_messages.push(std::move(owned_message));
                        }
                    }
                    this->m_context.m_cv.notify_one();
                }
            }
        }

        /*
            Queue message writing job.
        */
        void write_message() override final {
            auto shared = this->shared_from_this();

            asio::post(*m_context.io_context, [shared] {
                {
                    std::lock_guard<std::mutex> guard(shared->m_outgoing_mutex);

                    if (shared->m_send_message) {
                        return;
                    }

                    if (shared->m_outgoing_system_messages.empty() && shared->m_outgoing_messages.empty()) {
                        return;
                    }

                    if (!shared->m_outgoing_system_messages.empty()) {
                        shared->m_send_message = shared->m_outgoing_system_messages.front();
                        shared->m_outgoing_system_messages.pop();
                    }
                    else if (!shared->m_outgoing_messages.empty()) {
                        shared->m_send_message = shared->m_outgoing_messages.front();
                        shared->m_outgoing_messages.pop();
                    }
                    else {
                        shared->m_send_message = nullptr;
                    }
                }

                if (!shared->m_send_message)
                    return;

                if (shared->m_send_message->data_head.empty()) {
                    shared->m_send_message->head.send_timestamp =
                        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

                    // Pre process message data
                    shared->m_context.pre_process_message(shared->m_send_message->data);

                    // Set new data size
                    shared->m_send_message->head.data_size = shared->m_send_message->data.size();

                    // Serialize head
                    shared->m_send_message->head.serialize(shared->m_send_message->data_head);
                }

                return shared->write_message_head();
            });
        }
    };
}
