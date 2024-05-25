#pragma once

#include "libnetwrk/net/core/client/client_mngr_callback.hpp"
#include "libnetwrk/net/core/client/client_connection_internal.hpp"

#include <memory>

namespace libnetwrk {
    template<typename tn_desc, typename tn_socket>
    class client_mngr_connections : public client_mngr_callback<tn_desc, tn_socket> {
    public:
        using connection_t       = client_connection_internal<tn_desc, tn_socket>;
        using endpoint_t         = typename tn_socket::endpoint_t;
        using message_t          = connection_t::message_t;
        using owned_message_t    = connection_t::owned_message_t;
        using outgoing_message_t = connection_t::outgoing_message_t;

    protected:
        std::shared_ptr<connection_t> m_connection;

    protected:
        void create_connection() {
            this->m_connection = std::make_shared<connection_t>(*this->m_io_context);
        }

        void establish_connection(const endpoint_t& endpoint) {
            if (!this->m_connection) return;
            this->m_connection->connect(endpoint);
        }
    };
}