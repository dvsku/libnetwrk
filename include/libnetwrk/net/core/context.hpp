#pragma once

#include "libnetwrk/net/core/messages/owned_message.hpp"
#include "libnetwrk/net/core/containers/tsdeque.hpp"

#include <string>

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

    template<typename Command, typename Serialize, typename Storage>
    class context {
    public:
        using context_t         = asio::io_context;
        using owned_message_t   = owned_message<Command, Serialize, Storage>;
        using base_connection_t = base_connection<Command, Serialize, Storage>;

    public:
        const std::string   name;
        const context_owner owner;

        std::unique_ptr<context_t> asio_context;
        tsdeque<owned_message_t>   incoming_messages;

    public:
        context(const std::string& name, context_owner owner)
            : name(name), owner(owner) {}

    public:
        // Called when processing messages
        virtual void ev_message(owned_message_t& msg) {};

        virtual void internal_ev_client_disconnected(std::shared_ptr<base_connection_t> client) = 0;

    public:
        /// <summary>
        /// Processes a single message if the queue is not empty.
        /// </summary>
        /// <returns>true if a message has been processed, false if it hasn't</returns>
        bool process_message() {
            try {
                if (incoming_messages.empty())
                    return false;

                owned_message_t msg = incoming_messages.pop_front();
                ev_message(msg);
            }
            catch (const std::exception& e) {
                LIBNETWRK_ERROR(name, "process_message() fail | {}", e.what());
                return false;
            }
            catch (...) {
                LIBNETWRK_ERROR(name, "process_message() fail | undefined reason");
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

    private:
        void impl_process_messages() {
            while (m_status == service_status::started) {
                incoming_messages.wait();

                while (!incoming_messages.empty()) {
                    owned_message_t msg = incoming_messages.pop_front();
                    ev_message(msg);
                }
            }
        }
    };
}
