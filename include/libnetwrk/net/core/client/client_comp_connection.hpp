#pragma once

#include "libnetwrk/net/core/client/client_connection_internal.hpp"

#include <memory>

namespace libnetwrk {
    template<typename Context>
    class client_comp_connection {
    public:
        using context_t          = Context;
        using connection_t       = typename Context::connection_internal_t;
        using endpoint_t         = typename Context::endpoint_t;
        using message_t          = connection_t::message_t;
        using owned_message_t    = connection_t::owned_message_t;
        using outgoing_message_t = connection_t::outgoing_message_t;

    public:
        std::shared_ptr<connection_t> connection;

    public:
        client_comp_connection(context_t& context)
            : m_context(context) {}

    public:
        void create_connection() {
            connection = std::make_shared<connection_t>(m_context.io_context);
        }

        void establish_connection(const endpoint_t& endpoint) {
            if (connection)
                connection->connect(endpoint);
        }

    private:
        context_t& m_context;
    };
}