#pragma once

#include "libnetwrk/net/misc/authentication.hpp"

namespace libnetwrk {
    template<typename Context>
    class service_comp_system_message {
    public:
        using context_t             = Context;
        using connection_internal_t = context_t::connection_internal_t;
        using message_t             = context_t::message_t;
        using owned_message_t       = context_t::owned_message_t;

    public:
        service_comp_system_message(context_t& context)
            : m_context(context)
        {
            m_context.cb_system_message = [this](auto command, auto message) {
                ev_system_message(command, message);
            };
        }

        void send_auth_message(std::shared_ptr<connection_internal_t> connection) {
            connection->auth_request = authentication::generate_request();

            message_t request{};
            request.head.type    = message_type::system;
            request.head.command = static_cast<uint64_t>(system_command::s2c_verify);
            request << connection->auth_request;

            connection->send(request);

            auto deadline                     = std::chrono::system_clock::now() + std::chrono::seconds(m_context.settings.auth_deadline_sec);
            connection->auth_request_deadline = std::chrono::system_clock::to_time_t(deadline);
        }

    private:
        context_t& m_context;

    private:
        void ev_system_message(system_command command, owned_message_t* message) {
            switch (command) {
                case system_command::c2s_verify:     return on_system_verify_message(message);
                case system_command::cev_clock_sync: return on_system_clock_sync_message(message);
                
                default: return;
            }
        }

        void on_system_verify_message(owned_message_t* message) {
            LIBNETWRK_DEBUG(m_context.name, "Received verify response.");

            auto sender = std::static_pointer_cast<connection_internal_t>(message->sender);

            authentication::response_t auth_response{};
            message->message >> auth_response;

            if (!authentication::validate(sender->auth_request, auth_response))
                return sender->stop();

            sender->is_authenticated = true;

            message_t response{};
            response.head.type    = message_type::system;
            response.head.command = static_cast<uint64_t>(system_command::s2c_verify_ok);

            sender->send(response);
        }

        void on_system_clock_sync_message(owned_message_t* message) {
            LIBNETWRK_DEBUG(m_context.name, "Received clock sync request.");

            uint8_t  sample_index     = 0U;
            uint64_t client_timestamp = 0U;

            message->message >> sample_index >> client_timestamp;

            message_t response{};
            response.head.type    = message_type::system;
            response.head.command = static_cast<uint64_t>(system_command::sev_clock_sync);
            response << sample_index << client_timestamp << get_milliseconds_timestamp();

            message->sender->send(response);
        }
    };
}