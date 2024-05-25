#pragma once

#include "asio.hpp"
#include "libnetwrk/net/core/messages/owned_message.hpp"
#include "libnetwrk/net/core/messages/outgoing_message.hpp"
#include "libnetwrk/net/core/exceptions/libnetwrk_exception.hpp"

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <exception>

namespace libnetwrk {
    template<typename tn_mngr_connections>
    class shared_message_mngr : public tn_mngr_connections {
    public:
        using connection_t = typename tn_mngr_connections::connection_t;
        using command_t = connection_t::command_t;
        using message_t = connection_t::message_t;
        using owned_message_t = connection_t::owned_message_t;
        using outgoing_message_t = connection_t::outgoing_message_t;

    public:
        bool process_message() {
            {
                std::lock_guard<std::mutex> guard(m_incoming_mutex);

                if (m_incoming_system_messages.empty() && m_incoming_messages.empty())
                    return false;
            }

            return invoke_processing_callbacks();
        }

        bool process_messages() {
            process_messages_loop();
            return true;
        }

        bool process_messages_async() {
            m_process_messages_thread = std::thread([&] { process_messages_loop(); });
            return true;
        }

    protected:
        void start_connection_read_and_write(std::shared_ptr<connection_t> connection) {
            using namespace asio::experimental::awaitable_operators;

            asio::co_spawn(*this->m_io_context, this->co_read(connection) || connection->cancel_cv.wait(),
                [this, connection](auto, auto) {
                    connection->active_operations--;
                    LIBNETWRK_DEBUG(this->m_name, "{}: Stopped reading messages.", connection->get_id());
                }
            );

            asio::co_spawn(*this->m_io_context, this->co_write(connection) || connection->cancel_cv.wait(),
                [this, connection](auto, auto) {
                    connection->active_operations--;
                    LIBNETWRK_DEBUG(this->m_name, "{}: Stopped writing messages.", connection->get_id());
                }
            );
        }

        void stop_processing_messages() {
            {
                std::lock_guard<std::mutex> guard(m_incoming_mutex);

                m_incoming_messages = {};
                m_incoming_system_messages = {};
            }

            m_cv.notify_all();

            if (m_process_messages_thread.joinable())
                m_process_messages_thread.join();
        }

        asio::awaitable<void> co_read(std::shared_ptr<connection_t> connection) {
            std::error_code ec = {};

            connection->active_operations++;
            LIBNETWRK_DEBUG(this->m_name, "Started reading messages.");

            while (true) {
                if (!connection->is_connected())
                    break;

                owned_message_t owned_message;

                co_await connection->co_read_message(owned_message.msg, ec);

                if (ec) {
                    if (ec != asio::error::eof && ec != asio::error::connection_reset) {
                        LIBNETWRK_ERROR(this->m_name, "Failed during read. | {}", ec.message());
                    }

                    connection->stop();
                    if (this->m_ev_internal_disconnect_callback)
                        this->m_ev_internal_disconnect_callback();

                    break;
                }

                owned_message.sender = connection;

                // Post process message data
                if (this->m_ev_post_process_message_callback) {
                    this->m_ev_post_process_message_callback(&owned_message.msg.data);
                    owned_message.msg.head.data_size = owned_message.msg.data.size();
                }

                {
                    std::lock_guard<std::mutex> guard(this->m_incoming_mutex);

                    {
                        std::lock_guard<std::mutex> cv_lock(this->m_cv_mutex);

                        if (owned_message.msg.head.type == message_type::system) {
                            this->m_incoming_system_messages.push(std::move(owned_message));
                        }
                        else {
                            this->m_incoming_messages.push(std::move(owned_message));
                        }
                    }

                    this->m_cv.notify_one();
                }
            }
        }

        asio::awaitable<void> co_write(std::shared_ptr<connection_t> connection) {
            std::error_code ec = {};

            connection->active_operations++;
            LIBNETWRK_DEBUG(this->m_name, "Started writing messages.");

            while (true) {
                if (!connection->is_connected())
                    break;

                if (connection->wait_for_messages())
                    co_await connection->write_cv.wait();

                if (!connection->is_connected())
                    break;

                while (true) {
                    std::shared_ptr<outgoing_message_t> send_message;

                    {
                        std::lock_guard<std::mutex> guard(connection->get_outgoing_mutex());

                        auto& user_messages   = connection->get_user_messages();
                        auto& system_messages = connection->get_system_messages();

                        if (connection->has_system_messages()) {
                            send_message = system_messages.front();
                            system_messages.pop();
                        }
                        else if (connection->has_user_messages()) {
                            send_message = user_messages.front();
                            user_messages.pop();
                        }
                    }

                    if (!send_message)
                        break;

                    {
                        std::lock_guard<std::mutex> guard(send_message->mutex);

                        if (send_message->serialized_head.empty()) {
                            send_message->message.head.send_timestamp =
                                std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

                            // Pre process message data
                            if (this->m_ev_pre_process_message_callback) {
                                this->m_ev_pre_process_message_callback(&send_message->message.data);
                                send_message->message.head.data_size = send_message->message.data.size();
                            }

                            // Serialize head
                            send_message->message.head.serialize(send_message->serialized_head);
                        }
                    }

                    co_await connection->co_write_message(send_message, ec);

                    if (ec) {
                        if (ec != asio::error::eof && ec != asio::error::connection_reset) {
                            LIBNETWRK_ERROR(this->m_name, "Failed during write. | {}", ec.message());
                        }

                        connection->stop();
                        if (this->m_ev_internal_disconnect_callback)
                            this->m_ev_internal_disconnect_callback();

                        break;
                    }
                }

                if (ec)
                    break;
            }
        }

    private:
        std::queue<owned_message_t> m_incoming_messages;
        std::queue<owned_message_t> m_incoming_system_messages;
        std::mutex                  m_incoming_mutex;
        std::condition_variable     m_cv;
        std::mutex                  m_cv_mutex;
        std::thread                 m_process_messages_thread;

    private:
        void process_messages_loop() {
            while (this->m_status == service_status::started) {
                bool wait = false;

                {
                    std::lock_guard<std::mutex> guard(m_incoming_mutex);
                    wait = m_incoming_system_messages.empty() && m_incoming_messages.empty();
                }

                if (wait) {
                    std::unique_lock lock(m_cv_mutex);
                    m_cv.wait(lock);
                }

                if (this->m_status != service_status::started)
                    break;

                invoke_processing_callbacks();
            }
        }

        bool invoke_processing_callbacks() {
            try {
                owned_message_t message;

                {
                    std::lock_guard<std::mutex> guard(m_incoming_mutex);

                    if (!m_incoming_system_messages.empty()) {
                        message = m_incoming_system_messages.front();
                        m_incoming_system_messages.pop();
                    }
                    else {
                        message = m_incoming_messages.front();
                        m_incoming_messages.pop();
                    }
                }

                if (message.msg.head.type == message_type::system) {
                    if (!this->m_ev_system_message_callback)
                        throw libnetwrk_exception("System message callback not set.");

                    this->m_ev_system_message_callback(static_cast<system_command>(message.msg.head.command), &message);
                }
                else {
                    if (!this->m_ev_message_callback)
                        throw libnetwrk_exception("Message callback not set.");

                    this->m_ev_message_callback(message.msg.command(), &message);
                }
            }
            catch (const std::exception& e) {
                (void)e;

                LIBNETWRK_ERROR(this->m_name, "Failed to process message. | {}", e.what());
                return false;
            }
            catch (...) {
                LIBNETWRK_ERROR(this->m_name, "Failed to process message. | Critical fail.");
                return false;
            }

            return true;
        }
    };
}
