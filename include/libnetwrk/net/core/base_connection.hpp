#pragma once

#include "libnetwrk/net/core/context.hpp"
#include "libnetwrk/net/core/messages/owned_message.hpp"

#include <chrono>
#include <mutex>
#include <queue>

namespace libnetwrk {
    template<typename Desc, typename Socket>
    class base_connection  {
    public:
        // This connection type
        using connection_t = base_connection<Desc, Socket>;

        // Socket type
        using socket_t = Socket;

        // Message type
        using message_t = message<Desc>;

        // Serializer type
        using serialize_t = typename Desc::serialize_t;

    public:
        bool is_authenticated = false;

    public:
        base_connection()                       = delete;
        base_connection(const base_connection&) = delete;
        base_connection(base_connection&&)      = default;

        base_connection(socket_t socket)
            : m_socket(std::move(socket))
        {
            m_recv_message.data_head.resize(m_recv_message.head.size());
        }

        connection_t& operator=(const connection_t&) = delete;
        connection_t& operator=(connection_t&&)      = default;

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
            /*if (m_context.owner == context_owner::server) {
                m_verification_code = auth::generate_auth_question();

                message_t request;
                request.head.type    = message_type::system;
                request.head.command = static_cast<uint64_t>(system_command::s2c_verify);
                request << m_verification_code;

                send(request);
            }*/

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
        socket_t m_socket;
        uint64_t m_id = 0U;

        std::queue<std::shared_ptr<message_t>> m_outgoing_messages;
        std::queue<std::shared_ptr<message_t>> m_outgoing_system_messages;
        std::mutex                             m_outgoing_mutex;

        message_t                  m_recv_message;
        std::shared_ptr<message_t> m_send_message;

    protected:
        /*
            Called when there's a disconnect during read/write
        */
        virtual void internal_disconnect() {}

        /*
            Called when there's a failure during read/write
        */
        virtual void internal_failure(std::error_code ec) {}

        /*
            Called when finished reading a message
        */
        virtual void internal_read_callback() {}

        /*
            Queue message reading job
        */
        virtual void read_message() {}

        /*
            Queue message writing job
        */
        virtual void write_message() {}

    protected:
        virtual void read_message_head() {
            m_socket.async_read<serialize_t>(m_recv_message.data_head,
                std::bind(&connection_t::read_message_head_callback, this, std::placeholders::_1, std::placeholders::_2));
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
                    internal_read_callback();
                }
            }
            else if (ec == asio::error::eof || ec == asio::error::connection_reset) {
                internal_disconnect();
            }
            else {
                internal_failure(ec);
            }
        }

        virtual void read_message_body() {
            m_socket.async_read<serialize_t>(m_recv_message.data,
                std::bind(&connection_t::read_message_body_callback, this, std::placeholders::_1, std::placeholders::_2));
        };

        void read_message_body_callback(std::error_code ec, std::size_t len) {
            if (!ec) {
                internal_read_callback();
            }
            else if (ec == asio::error::eof || ec == asio::error::connection_reset) {
                internal_disconnect();
            }
            else {
                internal_failure(ec);
            }
        }

        virtual void write_message_head() {
            m_socket.async_write<serialize_t>(m_send_message->data_head,
                std::bind(&connection_t::write_message_head_callback, this, std::placeholders::_1, std::placeholders::_2));
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
            else if (ec == asio::error::eof || ec == asio::error::connection_reset) {
                internal_disconnect();
            }
            else {
                internal_failure(ec);
            }
        }

        virtual void write_message_body() {
            m_socket.async_write<serialize_t>(m_send_message->data,
                std::bind(&connection_t::write_message_body_callback, this, std::placeholders::_1, std::placeholders::_2));
        };

        void write_message_body_callback(std::error_code ec, std::size_t len) {
            if (!ec) {
                write_message();
            }
            else if (ec == asio::error::eof || ec == asio::error::connection_reset) {
                internal_disconnect();
            }
            else {
                internal_failure(ec);
            }
        }
    };
}
