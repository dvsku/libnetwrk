#pragma once

#include "libnetwrk/net/core/service/service_context.hpp"
#include "libnetwrk/net/core/service/service_comp_connection.hpp"
#include "libnetwrk/net/core/shared/shared_comp_message.hpp"
#include "libnetwrk/net/core/enums.hpp"

#include <functional>

namespace libnetwrk {
    template<typename tn_context>
    class service_comp_message : public shared_comp_message<tn_context> {
    public:
        using context_t          = tn_context;
        using comp_connection_t  = service_comp_connection<tn_context>;
        using connection_t       = context_t::connection_t;
        using message_t          = context_t::message_t;
        using outgoing_message_t = context_t::outgoing_message_t;

        using send_predicate_t   = std::function<bool(std::shared_ptr<connection_t>)>;

    public:
        service_comp_message(context_t& context, comp_connection_t& comp_connection)
            : shared_comp_message<tn_context>(context),
              m_comp_connection(comp_connection)
        {}

    public:
        void send(std::shared_ptr<connection_t> client, message_t& message, libnetwrk::send_flags flags) {
            if (!client || client->is_connected()) return;

            client->send(message, flags);
        }

        void send_all(message_t& message, libnetwrk::send_flags flags, send_predicate_t predicate) {
            std::shared_ptr<outgoing_message_t> outgoing_message;

            if (LIBNETWRK_FLAG_SET(flags, libnetwrk::send_flags::keep_message)) {
                outgoing_message = std::make_shared<outgoing_message_t>(message);
            }
            else {
                outgoing_message = std::make_shared<outgoing_message_t>(std::move(message));
            }

            std::lock_guard<std::mutex> guard(m_comp_connection.connections_mutex);
            for (auto& client : m_comp_connection.connections) {
                if (!client || !client->is_connected()) continue;
                if (predicate && !predicate(client))    continue;

                client->direct_send(outgoing_message);
            }
        }

    private:
        comp_connection_t& m_comp_connection;
    };
}