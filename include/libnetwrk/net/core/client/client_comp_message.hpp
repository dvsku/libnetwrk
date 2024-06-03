#pragma once

#include "libnetwrk/net/core/shared/shared_comp_message.hpp"
#include "libnetwrk/net/core/client/client_comp_connection.hpp"
#include "libnetwrk/net/enum/enums.hpp"

namespace libnetwrk {
    template<typename Context>
    class client_comp_message : public shared_comp_message<Context> {
    public:
        using context_t         = Context;
        using comp_connection_t = client_comp_connection<Context>;
        using message_t         = context_t::message_t;

    public:
        client_comp_message(context_t& context, comp_connection_t& comp_connection)
            : shared_comp_message<Context>(context),
              m_comp_connection(comp_connection)
        {}

    public:
        void send(message_t& message, libnetwrk::send_flags flags) {
            if (!m_comp_connection.connection || !m_comp_connection.connection->is_connected() || !this->m_context.is_running())
                return;

            m_comp_connection.connection->send(message, flags);
        }

    private:
        comp_connection_t& m_comp_connection;
    };
}
