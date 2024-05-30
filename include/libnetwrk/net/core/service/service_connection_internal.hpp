#pragma once

#include "libnetwrk/net/core/service/service_connection.hpp"
#include "libnetwrk/net/misc/authentication.hpp"

#include <atomic>

namespace libnetwrk {
    template<typename tn_desc, typename tn_socket>
    class service_connection_internal : public service_connection<tn_desc, tn_socket> {
    public:
        using base_t             = service_connection<tn_desc, tn_socket>;
        using io_context_t       = base_t::io_context_t;
        using command_t          = base_t::command_t;
        using connection_t       = service_connection_internal<tn_desc, tn_socket>;
        using endpoint_t         = typename tn_socket::endpoint_t;
        using message_t          = base_t::message_t;
        using owned_message_t    = base_t::owned_message_t;
        using outgoing_message_t = base_t::outgoing_message_t;

    public:
        service_connection_internal()                    = delete;
        service_connection_internal(const connection_t&) = delete;
        service_connection_internal(connection_t&&)      = default;

        service_connection_internal(io_context_t& context)
            : base_t(context), write_cv(context), cancel_cv(context)
        {
            is_authenticated      = false;
            auth_request          = {};
            auth_request_deadline = 0U;
            disconnect_code       = libnetwrk::disconnect_code::unspecified;
        }

        connection_t& operator=(const connection_t&) = delete;
        connection_t& operator=(connection_t&&)      = default;

    public:
        std::atomic_bool           is_authenticated;
        authentication::request_t  auth_request;
        uint64_t                   auth_request_deadline;
        libnetwrk::disconnect_code disconnect_code;

        coroutine_cv write_cv;
        coroutine_cv cancel_cv;

    public:
        bool wait_for_messages() {
            std::unique_lock<std::mutex> lock(this->m_outgoing_mutex);
            return this->m_outgoing_system_messages.empty() && this->m_outgoing_messages.empty();
        }

        bool has_user_messages() {
            return !this->m_outgoing_messages.empty();
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

        void direct_send(const std::shared_ptr<outgoing_message_t> outgoing_message) override final {
            base_t::direct_send(outgoing_message);
        }

    public:
        void connect(const endpoint_t& endpoint) {
            this->m_socket.connect(endpoint);
        }

        tn_socket& get_socket() {
            return this->m_socket;
        }

        void set_id(uint64_t id) {
            this->m_id = id;
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