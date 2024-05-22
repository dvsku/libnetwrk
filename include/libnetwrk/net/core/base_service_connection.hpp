#pragma once

#include "libnetwrk/net/core/base_connection.hpp"
#include "libnetwrk/net/core/auth.hpp"
#include "libnetwrk/net/core/system_commands.hpp"

#include <memory>

namespace libnetwrk {
    template<typename Desc, typename Socket>
    class base_client;

    /*
        Base connection that represents a client's service connection
    */
    template<typename Desc, typename Socket>
    class base_service_connection
        : public base_connection<Desc, Socket>, public std::enable_shared_from_this<base_service_connection<Desc, Socket>>
    {
    public:
        // This connection type
        using connection_t = base_service_connection<Desc, Socket>;
        
        // Socket type
        using socket_t = Socket;

        // Message type
        using message_t = message<Desc>;

        // Owned message type for this connection
        using owned_message_t = owned_message<Desc, connection_t>;

        // Context type for this connection
        using context_t = context<Desc, connection_t>;

        friend base_client<Desc, Socket>;

    public:
        base_service_connection()                               = delete;
        base_service_connection(const base_service_connection&) = delete;
        base_service_connection(base_service_connection&&)      = default;

        base_service_connection(context_t& context, socket_t socket)
            : base_connection<Desc, Socket>(std::move(socket)), m_context(context) {}

        base_service_connection& operator=(const base_service_connection&) = delete;
        base_service_connection& operator=(base_service_connection&&)      = default;

    public:
        /*
            Start read/write operations.
        */
        void start() override final {
            asio::co_spawn(*m_context.io_context, co_read(), asio::detached);
        }

    protected:
        context_t& m_context;

    private:
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

        asio::awaitable<void> co_write() {
            std::error_code ec;


            while (true) {
                if (!this->is_connected())
                    break;

                std::shared_ptr<message_t> send_message;

                {
                    std::lock_guard<std::mutex> guard(this->m_outgoing_mutex);

                    if (this->m_outgoing_system_messages.empty() && this->m_outgoing_messages.empty()) {
                        break;
                    }

                    /*
                        If not authenticated, write only system messages and
                        keep the user messages in the queue
                    */

                    if (!this->m_outgoing_system_messages.empty()) {
                        send_message = this->m_outgoing_system_messages.front();
                        this->m_outgoing_system_messages.pop();
                    }
                    else if (!this->m_outgoing_messages.empty() && this->is_authenticated.load()) {
                        send_message = this->m_outgoing_messages.front();
                        this->m_outgoing_messages.pop();
                    }
                }

                if (!send_message)
                    break;

                // TODO: Add mutex here

                {
                    if (send_message->data_head.empty()) {
                        send_message->head.send_timestamp =
                            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

                        // Pre process message data
                        m_context.pre_process_message(send_message->data);

                        // Set new data size
                        send_message->head.data_size = send_message->data.size();

                        // Serialize head
                        send_message->head.serialize(send_message->data_head);
                    }
                }

                co_await this->co_write_message(send_message, ec);

                if (ec) {
                    if (ec != asio::error::eof && ec != asio::error::connection_reset) {
                        LIBNETWRK_ERROR(this->m_context.name, "Failed during write. | {}", ec.message());
                    }

                    this->stop();
                    m_context.internal_ev_client_disconnected(this->shared_from_this());
                    break;
                }
            }
        }

        /*
            Queue message writing job.
        */
        void write_message() override final {
            asio::co_spawn(*m_context.io_context, co_write(), asio::detached);
        }
    };
}
