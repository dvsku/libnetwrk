#pragma once

#include "asio.hpp"
#include "libnetwrk/net/messages/message.hpp"
#include "libnetwrk/net/messages/outgoing_message.hpp"
#include "libnetwrk/net/misc/timestamp.hpp"
#include "libnetwrk/net/enum/enums.hpp"

#include <string>
#include <queue>
#include <mutex>

namespace libnetwrk {
    template<typename Desc, typename Socket>
    class shared_connection {
    public:
        using socket_t           = Socket;
        using io_context_t       = typename socket_t::io_context_t;
        using command_t          = typename Desc::command_t;
        using connection_t       = shared_connection<Desc, Socket>;
        using message_t          = message<Desc>;
        using outgoing_message_t = outgoing_message<Desc>;

    public:
        shared_connection()                    = delete;
        shared_connection(const connection_t&) = delete;
        shared_connection(connection_t&&)      = default;

        shared_connection(io_context_t& context)
            : m_socket(context) {}

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
            Get connection id.
        */
        uint64_t get_id() {
            return m_id;
        }

        /*
            Check if connected.
        */
        bool is_connected() {
            return m_socket.is_connected();
        }

    public:
        void send(message_t& message, libnetwrk::send_flags flags = libnetwrk::send_flags::none) {
            std::shared_ptr<outgoing_message_t> outgoing_message;

            if (enum_has_flag(flags, libnetwrk::send_flags::keep_message)) {
                outgoing_message = std::make_shared<outgoing_message_t>(message);
            }
            else {
                outgoing_message = std::make_shared<outgoing_message_t>(std::move(message));
            }

            direct_send(outgoing_message);
        }

    public:
        virtual void stop() {
            m_socket.close();
        }

    protected:
        socket_t m_socket;
        uint64_t m_id = 0U;

        std::queue<std::shared_ptr<outgoing_message_t>> m_outgoing_messages;
        std::queue<std::shared_ptr<outgoing_message_t>> m_outgoing_system_messages;
        std::mutex                                      m_outgoing_mutex;

    protected:
        virtual void notify() {};

        virtual void direct_send(const std::shared_ptr<outgoing_message_t> outgoing_message) {
            std::lock_guard<std::mutex> guard(m_outgoing_mutex);

            if (outgoing_message->message.head.type == message_type::system) {
                m_outgoing_system_messages.push(outgoing_message);
            }
            else {
                m_outgoing_messages.push(outgoing_message);
            }

            notify();
        }

    protected:
        asio::awaitable<void> co_read_message(message_t& recv_message, std::error_code& ec) {
            fixed_buffer<message_t::message_head_t::size> head_buffer;

            auto [h_ec, h_size] = co_await m_socket.async_read(head_buffer);

            if (h_ec) {
                ec = h_ec;
                co_return;
            }

            recv_message.head.deserialize(head_buffer);

            if (recv_message.head.data_size != 0) {
                recv_message.data.underlying().resize(recv_message.head.data_size);

                auto [b_ec, b_size] = co_await m_socket.async_read(recv_message.data);

                if (b_ec) {
                    ec = b_ec;
                    co_return;
                }
            }

            ec = {};
        }

        asio::awaitable<void> co_write_message(std::shared_ptr<outgoing_message_t> message, std::error_code& ec) {
            std::vector<asio::const_buffer> buffers;
            buffers.push_back(asio::buffer(message->serialized_head.data(), message->serialized_head.size()));

            if (message->message.data.size() != 0) {
                buffers.push_back(asio::buffer(message->message.data.data(), message->message.data.size()));
            }
            
            auto [err, size] = co_await m_socket.async_write(buffers);

            if (err) {
                ec = err;
                co_return;
            }

            ec = {};
        }
    };
}
