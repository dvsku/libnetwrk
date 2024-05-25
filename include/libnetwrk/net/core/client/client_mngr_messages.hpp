#pragma once

#include "libnetwrk/net/core/shared/shared_message_mngr.hpp"
#include "libnetwrk/net/core/client/client_mngr_connections.hpp"
#include "libnetwrk/net/core/enums.hpp"

namespace libnetwrk {
    template<typename tn_desc, typename tn_socket>
    class client_mngr_messages : public shared_message_mngr<client_mngr_connections<tn_desc, tn_socket>> {
    public:
        using connection_t    = client_mngr_connections<tn_desc, tn_socket>::connection_t;
        using message_t       = connection_t::message_t;
        using owned_message_t = connection_t::owned_message_t;

    public:
        bool is_connected() {
            return this->is_running();
        }

        void send(message_t& message, libnetwrk::send_flags flags = libnetwrk::send_flags::none) {
            if (!this->m_connection || !this->m_connection->is_connected() || !is_connected())
                return;

            this->m_connection->send(message, flags);
        }
    };
}
