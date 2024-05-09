#pragma once

#include "libnetwrk/net/core/context.hpp"
#include "libnetwrk/net/core/messages/owned_message.hpp"

#include <chrono>
#include <map>
#include <mutex>
#include <queue>

namespace libnetwrk {
    template<typename Desc, typename Socket>
    class base_connection : public std::enable_shared_from_this<base_connection<Desc, Socket>> {
    public:
        using base_connection_t = base_connection<Desc>;
        using base_connection_t = base_connection<Desc, Socket>;
        using socket_t          = Socket;
        using base_context_t    = context<Desc, Socket>;
        using message_t         = message<Desc>;
        using owned_message_t   = owned_message<Desc, Socket>;
        using storage_t         = typename Desc::storage_t;
        using serialize_t       = typename Desc::serialize_t;

    public:
        base_connection()                       = delete;
        base_connection(const base_connection&) = delete;
        base_connection(base_connection&&)      = default;

        base_connection(base_context_t& context, socket_t socket)
            : m_context(context), m_socket(std::move(socket))
        {
            m_recv_message.data_head.resize(m_recv_message.head.size());
        }

        base_connection_t& operator=(const base_connection_t&) = delete;
        base_connection_t& operator=(base_connection_t&&)      = default;

    public:
        /*
            Get connection storage
        */
        virtual storage_t& get_storage() {
            return m_storage;
        };

    public:
        /*
            Get ip address
        */
        std::string get_ip() {
            return m_socket.get_ip();
        }

        /*
            Get port
        */
        uint16_t get_port() {
            return m_socket.get_port();
        }

        /*
            Check if connected
        */
        bool is_connected() {
            return m_socket.is_connected();
        }

        /*
            Get connection id
        */
        uint64_t& id() {
            return m_id;
        }

        /// <summary>
        /// Start reading connection messages 
        /// </summary>
        void start() {

        virtual const std::string remote_address() = 0;

        virtual const unsigned short remote_port() = 0;

        virtual void stop() {};
            read_message();
            write_message();
        }

        void stop() {
            m_socket.close();
        };

        /// <summary>
        /// Send message
        /// </summary>
        /// <param name="message">ptr to message</param>
        void send(const std::shared_ptr<message_t> message) {
            std::lock_guard<std::mutex> guard(this->m_outgoing_mutex);

            if (message->head.type == message_type::system) {
                m_outgoing_system_messages.push(message);
            }
            else {
                m_outgoing_messages.push(message);
            }

        }

        /// <summary>
        /// Send message. 
        /// Message object after sending should be considered in an undefined state and
        /// shouldn't be used further without reassigning.
        /// </summary>
        /// <param name="message">: message to send</param>
        void send(message_t& message) {
            send(std::make_shared<message_t>(std::move(message)));
        }

    protected:
        base_context_t& m_context;
        storage_t       m_storage;
        socket_t        m_socket;

        std::queue<std::shared_ptr<message_t>> m_outgoing_messages;
        std::queue<std::shared_ptr<message_t>> m_outgoing_system_messages;
        std::mutex                             m_outgoing_mutex;

        uint64_t m_id                = 0U;
        uint32_t m_verification_ans  = 0;        // Correct verification answer, server only
        uint32_t m_verification_code = 0;

        message_t                  m_recv_message;
        std::shared_ptr<message_t> m_send_message;

    protected:
        void read_message() {
            asio::post(*m_context.asio_context, [this] {
                read_message_head();
            });
        }

        virtual void read_message_head() {
            m_socket.async_read<serialize_t>(m_recv_message.data_head,
                std::bind(&base_connection_t::read_message_head_callback, this, std::placeholders::_1, std::placeholders::_2));
        };

        void read_message_head_callback(std::error_code ec, std::size_t len) {
            if (!ec) {
                m_recv_message.data_head.reset_read_offset();
                m_recv_message.head.deserialize(m_recv_message.data_head);

                m_recv_message.head.recv_timestamp =
                    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

                // MESSAGE HAS A BODY
                if (m_recv_message.head.data_size > 0) {
                    m_recv_message.data.resize(m_recv_message.head.data_size);
                    read_message_body();
                }
                else {    // MESSAGE HAS NO BODY
                    m_recv_message.data.clear();
                    add_message_to_queue();
                }
            }
            else if (ec == asio::error::eof || ec == asio::error::connection_reset)
                on_disconnect();
            else
                on_error(ec);
        }

        virtual void read_message_body() {
            m_socket.async_read<serialize_t>(m_recv_message.data,
                std::bind(&base_connection_t::read_message_body_callback, this, std::placeholders::_1, std::placeholders::_2));
        };

        void read_message_body_callback(std::error_code ec, std::size_t len) {
            if (!ec)
                add_message_to_queue();
            else if (ec == asio::error::eof || ec == asio::error::connection_reset)
                on_disconnect();
            else
                on_error(ec);
        }

        void write_message() {
            asio::post(*m_context.asio_context, [this] {
                {
                    std::lock_guard<std::mutex> guard(this->m_outgoing_mutex);

                    if (!m_outgoing_system_messages.empty()) {
                        this->m_send_message = this->m_outgoing_system_messages.front();
                        this->m_outgoing_system_messages.pop();
                    }
                    else if (!m_outgoing_messages.empty()) {
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

                        this->m_send_message->head.serialize(this->m_send_message->data_head);
                    }

                    return write_message_head();
                }

                write_message();
            });
        }

        virtual void write_message_head() {
            m_socket.async_write<serialize_t>(m_send_message->data_head,
                std::bind(&base_connection_t::write_message_head_callback, this, std::placeholders::_1, std::placeholders::_2));
        };

        void write_message_head_callback(std::error_code ec, std::size_t len) {
            if (!ec) {
                if (m_send_message->data.size() > 0) {
                    write_message_body();
                }
                else {
                    write_message();
                }
            }
            else if (ec == asio::error::eof || ec == asio::error::connection_reset)
                on_disconnect();
            else
                on_error(ec);
        }

        virtual void write_message_body() {
            m_socket.async_write<serialize_t>(m_send_message->data,
                std::bind(&base_connection_t::write_message_body_callback, this, std::placeholders::_1, std::placeholders::_2));
        };

        void write_message_body_callback(std::error_code ec, std::size_t len) {
            if (!ec) {
                write_message();
            }
            else if (ec == asio::error::eof || ec == asio::error::connection_reset)
                on_disconnect();
            else
                on_error(ec);
        }

        void add_message_to_queue() {
            owned_message_t owned_message;
            owned_message.msg.head = std::move(m_recv_message.head);
            owned_message.msg.data = std::move(m_recv_message.data);
            owned_message.sender   = this->shared_from_this();

            {
                std::lock_guard<std::mutex> guard(m_context.incoming_mutex);

                if (owned_message.msg.head.type == message_type::system) {
                    m_context.incoming_system_messages.push(std::move(owned_message));
                }
                else {
                    m_context.incoming_messages.push(std::move(owned_message));
                }
            }

            read_message();
        }

    private:
        void on_disconnect() {
            stop();
            m_context.internal_ev_client_disconnected(this->shared_from_this());
        }

        void on_error(std::error_code ec) {
            stop();
            m_context.internal_ev_client_disconnected(this->shared_from_this());
            LIBNETWRK_ERROR(this->m_context.name,
                "failed during read/write | {}", ec.message());
        }
    };
}
