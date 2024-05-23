#pragma once

#include "libnetwrk/net/core/context.hpp"
#include "libnetwrk/net/core/misc/coroutine_cv.hpp"
#include "libnetwrk/net/core/enums.hpp"
#include "libnetwrk/net/core/messages/owned_message.hpp"
#include "libnetwrk/net/core/messages/outgoing_message.hpp"

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

        // Outgoing message type
        using outgoing_message_t = outgoing_message<Desc>;

        // Serializer type
        using serialize_t = typename Desc::serialize_t;

    public:
        std::atomic_bool   is_authenticated;
        std::atomic_ushort active_operations;

    public:
        base_connection()                       = delete;
        base_connection(const base_connection&) = delete;
        base_connection(base_connection&&)      = default;

        base_connection(work_context& context, socket_t socket)
            : m_socket(std::move(socket)), m_write_cv(context), m_cancel_cv(context)
        {
            is_authenticated.store(false);
            active_operations.store(0U);
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
            if(is_connected())
                m_socket.close();

            m_write_cv.notify_all();
            m_cancel_cv.notify_all();
        };

        /*
            Send message.
        */
        void send(const std::shared_ptr<outgoing_message_t> outgoing_message) {
            {
                std::lock_guard<std::mutex> guard(m_outgoing_mutex);

                if (outgoing_message->message.head.type == message_type::system) {
                    m_outgoing_system_messages.push(outgoing_message);
                }
                else {
                    m_outgoing_messages.push(outgoing_message);
                }

                m_write_cv.notify_one();
            }
        }

        /*
            Send message.
        */
        void send(message_t& message, libnetwrk::send_flags flags = libnetwrk::send_flags::none) {
            std::shared_ptr<outgoing_message_t> outgoing_message;

            if (LIBNETWRK_FLAG_SET(flags, libnetwrk::send_flags::keep_message)) {
                outgoing_message = std::make_shared<outgoing_message_t>(message);
            }
            else {
                outgoing_message = std::make_shared<outgoing_message_t>(std::move(message));
            }

            send(outgoing_message);
        }

    protected:
        socket_t m_socket;
        uint64_t m_id = 0U;

        std::queue<std::shared_ptr<outgoing_message_t>> m_outgoing_messages;
        std::queue<std::shared_ptr<outgoing_message_t>> m_outgoing_system_messages;
        std::mutex                                      m_outgoing_mutex;
        coroutine_cv                                    m_write_cv;
        coroutine_cv                                    m_cancel_cv;

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

        asio::awaitable<void> co_write_message(std::shared_ptr<outgoing_message_t> message, std::error_code& ec) {
            auto [h_ec, h_size] = co_await m_socket.async_write(message->serialized_head);

            if (h_ec) {
                ec = h_ec;
                co_return;
            }

            if (message->message.data.size() == 0) {
                ec = {};
                co_return;
            }

            auto [b_ec, b_size] = co_await m_socket.async_write(message->message.data);

            if (b_ec) {
                ec = b_ec;
                co_return;
            }

            ec = {};
        }
    };
}
