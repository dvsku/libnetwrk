#pragma once

#include "libnetwrk/net/core/messages/owned_message.hpp"

#include <string>
#include <queue>
#include <mutex>

namespace libnetwrk {
    enum class service_status : uint8_t {
        stopped  = 0x0,
        starting = 0x1,
        started  = 0x2,
        stopping = 0x3
    };

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
            : name(name) {}

    public:
        /*
            Processes a single message if the queue is not empty.
        */
        bool process_message() {
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
        service_status m_status = service_status::stopped;

        std::queue<owned_message_t> m_incoming_messages;
        std::queue<owned_message_t> m_incoming_system_messages;
        std::mutex                  m_incoming_mutex;

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
                process_message();
            }
        }
    };
}
