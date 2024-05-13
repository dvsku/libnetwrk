#pragma once

#include "libnetwrk/net/core/base_connection.hpp"
#include "libnetwrk/net/core/auth.hpp"
#include "libnetwrk/net/core/system_commands.hpp"

#include <memory>

namespace libnetwrk {
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

    public:
        auth::question_t auth_question{};

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
            Get connection storage
        */
        storage_t& get_storage() {
            return m_storage;
        }

    public:
        /*
            Start read/write operations
        */
        void start() override final {
            auth_question = auth::generate_auth_question();

            message_t request;
            request.head.type    = message_type::system;
            request.head.command = static_cast<uint64_t>(system_command::s2c_verify);
            request << auth_question;

            this->send(request);

            read_message();
            write_message();
        }

    protected:
        context_t& m_context;
        storage_t  m_storage;

    private:
        /*
            Called when there's a disconnect during read/write
        */
        void internal_disconnect() override final {
            this->stop();
            this->m_context.internal_ev_client_disconnected(this->shared_from_this());
        }

        /*
            Called when there's a failure during read/write
        */
        void internal_failure(std::error_code ec) override final {
            internal_disconnect();
            LIBNETWRK_ERROR(this->m_context.name, "Failed during read/write. | {}", ec.message());
        }

        /*
            Called when finished reading a message
        */
        void internal_read_callback() override final {
            owned_message_t owned_message;
            owned_message.msg.head = std::move(this->m_recv_message.head);
            owned_message.msg.data = std::move(this->m_recv_message.data);
            owned_message.sender   = this->shared_from_this();

            /*
                If client is not authenticated, discard user messages
            */

            if (!this->is_authenticated.load() && owned_message.msg.head.type != message_type::system)
                this->read_message();

            // Post process message data
            m_context.post_process_message(owned_message.msg.data);

            // Set new data size
            owned_message.msg.head.data_size = owned_message.msg.data.size();

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

                    if (!this->m_outgoing_system_messages.empty()) {
                        this->m_send_message = this->m_outgoing_system_messages.front();
                        this->m_outgoing_system_messages.pop();
                    }
                    else if (!this->m_outgoing_messages.empty()) {
                        this->m_send_message = this->m_outgoing_messages.front();
                        this->m_outgoing_messages.pop();
                    }
                    else {
                        this->m_send_message = nullptr;
                    }
                }

                if (this->m_send_message) {
                    if (this->m_send_message->data_head.empty()) {
                        this->m_send_message->head.send_timestamp =
                            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

                        // Pre process message data
                        m_context.pre_process_message(this->m_send_message->data);

                        // Set new data size
                        this->m_send_message->head.data_size = this->m_send_message->data.size();

                        // Serialize head
                        this->m_send_message->head.serialize(this->m_send_message->data_head);
                    }

                    return this->write_message_head();
                }

                write_message();
            });
        }
    };
}
