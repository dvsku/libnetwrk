#pragma once

#include "libnetwrk/net/core/client/client_mngr_messages.hpp"
#include "libnetwrk/net/core/auth.hpp"

namespace libnetwrk {
    template<typename tn_desc, typename tn_socket>
    class client_mngr_system_messages : public client_mngr_messages<tn_desc, tn_socket> {
    public:
        using base_t          = client_mngr_messages<tn_desc, tn_socket>;
        using connection_t    = base_t::connection_t;
        using message_t       = base_t::message_t;
        using owned_message_t = base_t::owned_message_t;

    public:
        client_mngr_system_messages() {
            this->set_system_message_callback([this](auto command, auto message) {
                ev_system_message(command, message);
            });
        }

    private:
        void ev_system_message(system_command command, owned_message_t* message) {
            switch (command) {
                case system_command::s2c_verify:    return on_system_verify_message(message);
                case system_command::s2c_verify_ok: return on_system_verify_ok_message(message);
                default:                            return;
            }
        }

        void on_system_verify_message(owned_message_t* message) {
            LIBNETWRK_DEBUG(this->m_name, "Received verify request.");

            auth::question_t question{};
            auth::answer_t   answer{};

            message->msg >> question;
            answer = auth::generate_auth_answer(question);

            message_t response{};
            response.head.type = message_type::system;
            response.head.command = static_cast<uint64_t>(system_command::c2s_verify);
            response << answer;

            this->send(response);
        }

        void on_system_verify_ok_message(owned_message_t* msg) {
            LIBNETWRK_DEBUG(this->m_name, "Verification successful.");

            this->m_connection->is_authenticated = true;
            this->m_connection->write_cv.notify_all();
        }
    };
}
