#pragma once

#include "libnetwrk/net/core/enums.hpp"
#include "libnetwrk/net/core/messages/owned_message.hpp"

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace libnetwrk {
    class work_context {
    public:
        using io_context_t = asio::io_context;

    public:
        std::unique_ptr<io_context_t> io_context;
    };

    template<typename Desc, typename Connection>
    class context : protected work_context {
    public:
        // Connection type for this context
        using base_connection_t = Connection;

        // Owned message type for this context
        using owned_message_t = owned_message<Desc, base_connection_t>;
        
        // Buffer type
        using buffer_t = owned_message_t::message_t::buffer_t;

    public:
        const std::string name;

        friend base_connection_t;

    public:
        context(const std::string& name)
            : name(name)
        {
            m_status = libnetwrk::service_status::stopped;
        }

    public:
        /*
            Processes a single message if the queue is not empty.
        */
        bool process_message() {
            {
                std::lock_guard<std::mutex> guard(this->m_incoming_mutex);

                if (this->m_incoming_system_messages.empty() && this->m_incoming_messages.empty())
                    return false;
            }

            return internal_process_message();
        }

        /*
            Process messages while running.
            This is a blocking function.
        */
        void process_messages() {
            impl_process_messages();
        }

        /*
            Process messages while running.
            This function runs asynchronously.
        */
        void process_messages_async() {
            m_process_messages_thread = std::thread([&] { impl_process_messages(); });
        }

    protected:
        std::atomic_uint8_t m_status;

        std::queue<owned_message_t> m_incoming_messages;
        std::queue<owned_message_t> m_incoming_system_messages;
        std::mutex                  m_incoming_mutex;
        std::condition_variable     m_cv;
        std::mutex                  m_cv_mutex;

        std::thread m_process_messages_thread;

    protected:
        /*
            Client disconnected callback from connection.
        */
        virtual void internal_ev_client_disconnected(std::shared_ptr<base_connection_t> client) = 0;

        /*
            Initial message processing.
        */
        virtual bool internal_process_message() {
            return false;
        }
  
        /*
            User message processing.
        */
        virtual void ev_message(owned_message_t& msg) {};

        /*
            System message processing.
        */
        virtual void ev_system_message(owned_message_t& msg) {}

        /*
            Pre process message data before writing.
        */
        virtual void pre_process_message(buffer_t& buffer) = 0;

        /*
            Post process message data after reading.
        */
        virtual void post_process_message(buffer_t& buffer) = 0;

    private:
        void impl_process_messages() {
            while (m_status == service_status::started) {
                bool wait = false;
                
                {
                    std::lock_guard<std::mutex> guard(this->m_incoming_mutex);
                    wait = this->m_incoming_system_messages.empty() && this->m_incoming_messages.empty();
                }

                if (wait) {
                    std::unique_lock lock(m_cv_mutex);
                    m_cv.wait(lock);
                }

                if (m_status != service_status::started)
                    break;

                internal_process_message();
            }
        }
    };
}
