#pragma once

#include "libnetwrk/net/core/base_connection.hpp"
#include "libnetwrk/net/core/auth.hpp"
#include "libnetwrk/net/core/system_commands.hpp"

#include <memory>

namespace libnetwrk {
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
            Start read/write operations
        */
        void start() override final {
            read_message();
            write_message();
        }

    protected:
        context_t& m_context;

    private:
        /*
            Called when there's a disconnect during read/write
        */
        void internal_disconnect() override {
            this->stop();
            this->m_context.internal_ev_client_disconnected(this->shared_from_this());
        }

        /*
            Called when there's a failure during read/write
        */
        void internal_failure(std::error_code ec) override {
            internal_disconnect();
            LIBNETWRK_ERROR(this->m_context.name, "Failed during read/write | {}", ec.message());
        }

        /*
            Called when finished reading a message
        */
        void internal_read_callback() override final {
            owned_message_t owned_message;
            owned_message.msg.head = std::move(this->m_recv_message.head);
            owned_message.msg.data = std::move(this->m_recv_message.data);
            owned_message.sender   = this->shared_from_this();

            {
                std::lock_guard<std::mutex> guard(this->m_context.incoming_mutex);

                if (owned_message.msg.head.type == message_type::system) {
                    this->m_context.incoming_system_messages.push(std::move(owned_message));
                }
                else {
                    this->m_context.incoming_messages.push(std::move(owned_message));
                }
            }

            this->read_message();
        }

        /*
            Queue message reading job
        */
        void read_message() override final {
            asio::post(*m_context.io_context, [this] {
                this->read_message_head();
            });
        }

        /*
            Queue message writing job
        */
        void write_message() override final {
            asio::post(*m_context.io_context, [this] {
                {
                    std::lock_guard<std::mutex> guard(this->m_outgoing_mutex);

                    /*
                        If not authenticated, write only system messages and
                        keep the user messages in the queue
                    */

                    if (!this->m_outgoing_system_messages.empty()) {
                        this->m_send_message = this->m_outgoing_system_messages.front();
                        this->m_outgoing_system_messages.pop();
                    }
                    else {
                        if (this->is_authenticated.load() && !this->m_outgoing_messages.empty()) {
                            this->m_send_message = this->m_outgoing_messages.front();
                            this->m_outgoing_messages.pop();
                        }
                    }
                }

                if (this->m_send_message) {
                    if (this->m_send_message->data_head.empty()) {
                        this->m_send_message->head.send_timestamp =
                            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

                        this->m_send_message->head.serialize(this->m_send_message->data_head);
                    }

                    return this->write_message_head();
                }

                write_message();
            });
        }
    };
}
