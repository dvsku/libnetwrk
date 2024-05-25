#pragma once

#include "libnetwrk/net/core/libnetwrk_context.hpp"
#include "libnetwrk/net/core/system_commands.hpp"
#include "libnetwrk/net/core/client/client_connection.hpp"

#include <functional>

namespace libnetwrk {
    template<typename tn_desc, typename tn_socket>
    class client_mngr_callback : public libnetwrk_context {
    public:
        using connection_t    = client_connection<tn_desc, tn_socket>;
        using command_t       = connection_t::command_t;
        using message_t       = connection_t::message_t;
        using owned_message_t = connection_t::owned_message_t;
        using buffer_t        = buffer<typename tn_desc::serialize_t>;

        using ev_message_callback_t              = std::function<void(command_t,      owned_message_t*)>;
        using ev_system_message_callback_t       = std::function<void(system_command, owned_message_t*)>;
        using ev_internal_disconnect_callback_t  = std::function<void()>;
        using ev_connect_callback_t              = std::function<void()>;
        using ev_disconnect_callback_t           = std::function<void()>;
        using ev_pre_process_message_callback_t  = std::function<void(buffer_t*)>;
        using ev_post_process_message_callback_t = std::function<void(buffer_t*)>;

    public:
        void set_message_callback(ev_message_callback_t cb) {
            // TODO: Check that status is stopped

            if (!m_ev_message_callback)
                m_ev_message_callback = cb;
        }

        void set_connect_callback(ev_connect_callback_t cb) {
            if (!m_ev_connect_callback)
                m_ev_connect_callback = cb;
        }

        void set_disconnect_callback(ev_disconnect_callback_t cb) {
            if (!m_ev_disconnect_callback)
                m_ev_disconnect_callback = cb;
        }

        void set_pre_process_message_callback(ev_pre_process_message_callback_t cb) {
            if (!m_ev_pre_process_message_callback)
                m_ev_pre_process_message_callback = cb;
        }

        void set_post_process_message_callback(ev_post_process_message_callback_t cb) {
            if (!m_ev_post_process_message_callback)
                m_ev_post_process_message_callback = cb;
        }

    protected:
        void set_system_message_callback(ev_system_message_callback_t cb) {
            // TODO: Check that status is stopped

            if (!m_ev_system_message_callback)
                m_ev_system_message_callback = cb;
        }

        void set_internal_disconnect_callback(ev_internal_disconnect_callback_t cb) {
            if (!m_ev_internal_disconnect_callback)
                m_ev_internal_disconnect_callback = cb;
        }

    protected:
        ev_message_callback_t              m_ev_message_callback;
        ev_system_message_callback_t       m_ev_system_message_callback;
        ev_internal_disconnect_callback_t  m_ev_internal_disconnect_callback;
        ev_connect_callback_t              m_ev_connect_callback;
        ev_disconnect_callback_t           m_ev_disconnect_callback;
        ev_pre_process_message_callback_t  m_ev_pre_process_message_callback;
        ev_post_process_message_callback_t m_ev_post_process_message_callback;
    };
}
