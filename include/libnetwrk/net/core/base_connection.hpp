#pragma once

#include "libnetwrk/net/core/context.hpp"
#include "libnetwrk/net/core/messages/owned_message.hpp"

#include <chrono>
#include <mutex>
#include <queue>
#include <atomic>

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
        std::atomic_bool   is_authenticated;
        std::atomic_ushort read_operations;
        std::atomic_ushort write_operations;

    public:
        base_connection()                       = delete;
        base_connection(const base_connection&) = delete;
        base_connection(base_connection&&)      = default;

        base_connection(socket_t socket)
            : m_socket(std::move(socket))
        {
            is_authenticated.store(false);
            read_operations.store(0U);
            write_operations.store(0U);
        }

        connection_t& operator=(const connection_t&) = delete;
        connection_t& operator=(connection_t&&)      = default;

    public:
        /*
            Get ip address.
        */
        std::string get_ip() {
            return m_socket.get_ip();
        }

        /*
            Get port.
        */
        uint16_t get_port() {
            return m_socket.get_port();
        }

        /*
            Check if connected.
        */
        bool is_connected() {
            return m_socket.is_connected();
        }

        /*
            Get connection id.
        */
        uint64_t& id() {
            return m_id;
        }

        /*
            Start read/write operations.
        */
        virtual void start() = 0;

        void stop() {
            m_socket.close();
        };

        /*
            Send message.
        */
        void send(const std::shared_ptr<message_t> message) {
            {
                std::lock_guard<std::mutex> guard(this->m_outgoing_mutex);

                if (message->head.type == message_type::system) {
                    m_outgoing_system_messages.push(message);
                }
                else {
                    m_outgoing_messages.push(message);
                }
            }
            
            write_message();
        }

        /*
            Send message.
        */
        void send(message_t& message) {
            send(std::make_shared<message_t>(std::move(message)));
        }

    protected:
        socket_t m_socket;
        uint64_t m_id = 0U;

        std::queue<std::shared_ptr<message_t>> m_outgoing_messages;
        std::queue<std::shared_ptr<message_t>> m_outgoing_system_messages;
        std::mutex                             m_outgoing_mutex;

    protected:
        /*
            Queue message writing job.
        */
        virtual void write_message() {}

    protected:
        asio::awaitable<void> co_read_message(message_t& recv_message, std::error_code& ec) {
            buffer<serialize_t> head_buffer;
            head_buffer.resize(message_t::message_head_t::size);

            auto [h_ec, h_size] = co_await m_socket.async_read(head_buffer);

            if (h_ec) {
                ec = h_ec;
                co_return;
            }

            recv_message.head.deserialize(head_buffer);

            recv_message.head.recv_timestamp =
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

            if (recv_message.head.data_size == 0) {
                ec = {};
                co_return;
            }

            recv_message.data.resize(recv_message.head.data_size);

            auto [b_ec, b_size] = co_await m_socket.async_read(recv_message.data);

            if (b_ec) {
                ec = b_ec;
                co_return;
            }

            ec = {};
        }

        asio::awaitable<void> co_write_message(std::shared_ptr<message_t> message, std::error_code& ec) {
            auto [h_ec, h_size] = co_await m_socket.async_write(message->data_head);

            if (h_ec) {
                ec = h_ec;
                co_return;
            }

            if (message->data.size() == 0) {
                ec = {};
                co_return;
            }

            auto [b_ec, b_size] = co_await m_socket.async_write(message->data);

            if (b_ec) {
                ec = b_ec;
                co_return;
            }

            ec = {};
        }
    };
}
