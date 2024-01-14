#pragma once

#include "libnetwrk/net/core/context.hpp"
#include "libnetwrk/net/core/containers/tsdeque.hpp"
#include "libnetwrk/net/core/messages/owned_message.hpp"

#include <chrono>
#include <map>
#include <mutex>
#include <random>

namespace libnetwrk {
    struct nothing {};

    template<typename Command, typename Serialize, typename Storage>
    class base_connection : public std::enable_shared_from_this<base_connection<Command, Serialize, Storage>> {
    public:
        using base_connection_t = base_connection<Command, Serialize, Storage>;
        using base_context_t    = context<Command, Serialize, Storage>;
        using message_t         = message<Command, Serialize>;
        using owned_message_t   = owned_message<Command, Serialize, Storage>;
        using storage_t         = Storage;

    public:
        base_connection()                       = delete;
        base_connection(const base_connection&) = delete;
        base_connection(base_connection&&)      = default;

        base_connection(base_context_t& context)
            : m_context(context) 
        {
            m_recv_message.data_head.resize(m_recv_message.head.size());
        }

        base_connection_t& operator=(const base_connection_t&) = delete;
        base_connection_t& operator=(base_connection_t&&)      = default;

    public:
        /// <summary>
        /// Get connection storage
        /// </summary>
        /// <returns>storage</returns>
        storage_t& get_storage() {
            return m_storage;
        }

        /// <summary>
        /// Get connection id
        /// </summary>
        uint64_t& id() {
            return m_id;
        }

        /// <summary>
        /// Start reading connection messages 
        /// </summary>
        void start() {
            if (m_context.owner == context_owner::server) {
                m_verification_code = generate_verification_code();
                m_verification_ans  = generate_verification_answer(m_verification_code);
                write_verification_message();
            }
            else {
                read_verification_message();
            }
        }

        virtual const std::string remote_address() = 0;

        virtual const unsigned short remote_port() = 0;

        virtual void stop() {};

        virtual bool is_alive() = 0;

        /// <summary>
        /// Send message
        /// </summary>
        /// <param name="message">ptr to message</param>
        void send(const std::shared_ptr<message_t> message) {
            asio::post(*m_context.asio_context, [this, message]() {
                bool was_empty = m_outgoing_messages.empty();
                m_outgoing_messages.push_back(message);

                if (was_empty)
                    write_message_head();
            });
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
        base_context_t&                     m_context;
        tsdeque<std::shared_ptr<message_t>> m_outgoing_messages;
        storage_t                           m_storage;

        uint64_t m_id                = 0U;
        uint32_t m_verification_ans  = 0;        // Correct verification answer, server only
        uint32_t m_verification_code = 0;

        message_t m_recv_message;

    protected:
        virtual void read_verification_message() = 0;

        virtual void read_message_head() = 0;

        virtual void read_message_body() = 0;

        virtual void write_verification_message() = 0;

        virtual void write_message_head() = 0;

        virtual void write_message_body() = 0;

        void read_verification_message_callback(std::error_code ec, std::size_t len) {
            if (!ec) {
                if (m_context.owner == context_owner::server) {
                    if (m_verification_code == m_verification_ans) {
                        read_message_head();
                    }
                    else {
                        LIBNETWRK_WARNING(this->m_context.name,
                            "client verification failed; disconnecting client");
                        stop();
                    }
                }
                else {
                    m_verification_code = generate_verification_answer(m_verification_code);
                    write_verification_message();
                }
            }
            else if (ec == asio::error::eof || ec == asio::error::connection_reset)
                on_disconnect();
            else
                on_error(ec);
        }

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

        void read_message_body_callback(std::error_code ec, std::size_t len) {
            if (!ec)
                add_message_to_queue();
            else if (ec == asio::error::eof || ec == asio::error::connection_reset)
                on_disconnect();
            else
                on_error(ec);
        }

        void write_verification_message_callback(std::error_code ec, std::size_t len) {
            if (!ec) {
                if (this->m_context.owner == context_owner::server)
                    read_verification_message();
                else
                    read_message_head();
            }
            else if (ec == asio::error::eof || ec == asio::error::connection_reset)
                on_disconnect();
            else
                on_error(ec);
        }

        void write_message_head_callback(std::error_code ec, std::size_t len) {
            if (!ec) {
                if (m_outgoing_messages.front()->data.size() > 0) {
                    write_message_body();
                }
                else {
                    m_outgoing_messages.pop_front();

                    if (!m_outgoing_messages.empty())
                        write_message_head();
                }
            }
            else if (ec == asio::error::eof || ec == asio::error::connection_reset)
                on_disconnect();
            else
                on_error(ec);
        }

        void write_message_body_callback(std::error_code ec, std::size_t len) {
            if (!ec) {
                m_outgoing_messages.pop_front();

                if (!m_outgoing_messages.empty())
                    write_message_head();
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

            owned_message.sender = this->shared_from_this();
            m_context.incoming_messages.emplace_back(std::move(owned_message));

            read_message_head();
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

        uint32_t generate_verification_code() {
            std::random_device seed;
            std::default_random_engine generator(seed());
            std::uniform_int_distribution<uint32_t> distribution(0x0000, 0xFFFF);
            return distribution(generator);
        }

        uint32_t generate_verification_answer(uint32_t request) {
            return request ^ 21205;
        }
    };
}
