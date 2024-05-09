#pragma once

#include "libnetwrk/net/core/messages/owned_message.hpp"

#include <string>
#include <queue>
#include <mutex>

namespace libnetwrk {
    enum class context_owner : uint8_t {
        server = 0x0,
        client = 0x1
    };

    enum class service_status : uint8_t {
        stopped  = 0x0,
        starting = 0x1,
        started  = 0x2,
        stopping = 0x3
    };

    template<typename Desc, typename Socket>
    class context {
    public:
        using context_t         = asio::io_context;
        using owned_message_t   = owned_message<Desc, Socket>;
        using base_connection_t = base_connection<Desc, Socket>;

    public:
        const std::string   name;
        const context_owner owner;

        std::unique_ptr<context_t> asio_context;

        std::queue<owned_message_t> incoming_messages;
        std::queue<owned_message_t> incoming_system_messages;
        std::mutex                  incoming_mutex;

    public:
        context(const std::string& name, context_owner owner)
            : name(name), owner(owner) {}

    public:
        virtual void internal_ev_client_disconnected(std::shared_ptr<base_connection_t> client) = 0;

    public:
        /// <summary>
        /// Processes a single message if the queue is not empty.
        /// </summary>
        /// <returns>true if a message has been processed, false if it hasn't</returns>
        bool process_message() {
            try {
                owned_message_t message;

                {
                    std::lock_guard<std::mutex> guard(incoming_mutex);

                    if (incoming_messages.empty())
                        return false;

                    message = incoming_messages.front();
                    incoming_messages.pop();
                }

                internal_process_message(message);
            }
            catch (const std::exception& e) {
                (void)e;

                LIBNETWRK_ERROR(name, "Failed to process message. | {}", e.what());
                return false;
            }
            catch (...) {
                LIBNETWRK_ERROR(name, "Failed to process message. | Critical fail");
                return false;
            }

            return true;
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
        virtual void internal_process_message(owned_message_t& msg) {}
  
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
