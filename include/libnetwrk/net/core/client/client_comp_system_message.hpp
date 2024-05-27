#pragma once

#include "libnetwrk/net/misc/authentication.hpp"

namespace libnetwrk {
    template<typename tn_context>
    class client_comp_system_message {
    public:
        using context_t             = tn_context;
        using connection_internal_t = context_t::connection_internal_t;
        using message_t             = context_t::message_t;
        using owned_message_t       = context_t::owned_message_t;

    public:
        client_comp_system_message(context_t& context)
            : m_context(context)
        {
            m_context.cb_system_message = [this](auto command, auto message) {
                ev_system_message(command, message);
            };
        }

    private:
        context_t& m_context;

    private:
        void ev_system_message(system_command command, owned_message_t* message) {
            switch (command) {
                case system_command::s2c_verify:    return on_system_verify_message(message);
                case system_command::s2c_verify_ok: return on_system_verify_ok_message(message);
                default:                            return;
            }
        }

        void on_system_verify_message(owned_message_t* message) {
            LIBNETWRK_DEBUG(m_context.name, "Received verify request.");

            authentication::request_t  auth_request{};
            authentication::response_t auth_response{};

            message->msg >> auth_request;
            auth_response = authentication::generate_response(auth_request);

            message_t response{};
            response.head.type    = message_type::system;
            response.head.command = static_cast<uint64_t>(system_command::c2s_verify);
            response << auth_response;

            message->sender->send(response);
        }

        void on_system_verify_ok_message(owned_message_t* msg) {
            LIBNETWRK_DEBUG(m_context.name, "Verification successful.");

            auto connection = std::static_pointer_cast<connection_internal_t>(msg->sender);
            connection->is_authenticated = true;
            connection->write_cv.notify_all();
        }
    };
}
