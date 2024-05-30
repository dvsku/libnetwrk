#pragma once

#include "libnetwrk/net/core/client/client_connection.hpp"
#include "libnetwrk/net/misc/coroutine_cv.hpp"
#include "libnetwrk/net/enum/enums.hpp"

#include <memory>
#include <atomic>

namespace libnetwrk {
    template<typename tn_desc, typename tn_socket>
    class client_connection_internal
        : public client_connection<tn_desc, tn_socket>,
          public std::enable_shared_from_this<client_connection_internal<tn_desc, tn_socket>> 
    {
    public:
        using base_t             = client_connection<tn_desc, tn_socket>;
        using io_context_t       = base_t::io_context_t;
        using command_t          = base_t::command_t;
        using connection_t       = client_connection_internal<tn_desc, tn_socket>;
        using endpoint_t         = typename tn_socket::endpoint_t;
        using message_t          = base_t::message_t;
        using owned_message_t    = base_t::owned_message_t;
        using outgoing_message_t = base_t::outgoing_message_t;

    public:
        client_connection_internal()                    = delete;
        client_connection_internal(const connection_t&) = delete;
        client_connection_internal(connection_t&&)      = default;

        client_connection_internal(io_context_t& context)
            : base_t(context), write_cv(context), cancel_cv(context)
        {
            is_authenticated  = false;
        }

        connection_t& operator=(const connection_t&) = delete;
        connection_t& operator=(connection_t&&)      = default;

    public:
        std::atomic_bool is_authenticated;

        coroutine_cv write_cv;
        coroutine_cv cancel_cv;

    public:
        bool wait_for_messages() {
            std::unique_lock<std::mutex> lock(this->m_outgoing_mutex);
            return this->m_outgoing_system_messages.empty() || (this->m_outgoing_messages.empty() && this->is_authenticated);
        }

        bool has_user_messages() {
            return !this->m_outgoing_messages.empty() && this->is_authenticated;
        }

        bool has_system_messages() {
            return !this->m_outgoing_system_messages.empty();
        }

        std::queue<std::shared_ptr<outgoing_message_t>>& get_user_messages()   { return this->m_outgoing_messages; }
        std::queue<std::shared_ptr<outgoing_message_t>>& get_system_messages() { return this->m_outgoing_system_messages; }
        std::mutex&                                      get_outgoing_mutex()  { return this->m_outgoing_mutex; }

    public:
        void stop() override final {
            base_t::stop();
            write_cv.notify_all();
            cancel_cv.notify_all();
        }

    public:
        void connect(const endpoint_t& endpoint) {
            this->m_socket.connect(endpoint);
        }

        asio::awaitable<void> co_read_message(message_t& recv_message, std::error_code& ec) {
            return base_t::base_t::co_read_message(recv_message, ec);
        }

        asio::awaitable<void> co_write_message(std::shared_ptr<outgoing_message_t> message, std::error_code& ec) {
            return base_t::base_t::co_write_message(message, ec);
        }

    protected:
        void notify() override final {
            write_cv.notify_one();
        }
    };
}
