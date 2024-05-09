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
    class context : public work_context {
    public:
        using owned_message_t   = owned_message<Desc, Connection>;
        using base_connection_t = Connection;

    public:
        const std::string name;

        std::queue<owned_message_t> incoming_messages;
        std::queue<owned_message_t> incoming_system_messages;
        std::mutex                  incoming_mutex;

    public:
        context(const std::string& name)
            : name(name) {}

    public:
        virtual void internal_ev_client_disconnected(std::shared_ptr<base_connection_t> client) = 0;

    public:
        /// <summary>
        /// Processes a single message if the queue is not empty.
        /// </summary>
        /// <returns>true if a message has been processed, false if it hasn't</returns>
        bool process_message() {
            return internal_process_message();
        }

        /// <summary>
        /// Process messages while running. This is a blocking function.
        /// </summary>
        void process_messages() {
            impl_process_messages();
        }

        /// <summary>
        /// Process messages while running. 
        /// This function runs asynchronously.
        /// </summary>
        void process_messages_async() {
            m_process_messages_thread = std::thread([&] { impl_process_messages(); });
        }

    protected:
        service_status m_status = service_status::stopped;
        std::thread    m_process_messages_thread;

    protected:
        /*
            Initial message processing
        */
        virtual bool internal_process_message() {
            return false;
        }
  
        /*
            User message processing
        */
        virtual void ev_message(owned_message_t& msg) {};

        /*
            System message processing
        */
        virtual void ev_system_message(owned_message_t& msg) {}

    private:
        void impl_process_messages() {
            while (m_status == service_status::started) {
                process_message();
            }
        }
    };
}
