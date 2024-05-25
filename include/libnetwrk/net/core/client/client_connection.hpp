#pragma once

#include "libnetwrk/net/core/shared/shared_connection.hpp"

namespace libnetwrk {
    template<typename tn_desc, typename tn_socket>
    class client_connection : public shared_connection<tn_desc, tn_socket> {
    public:
        using base_t             = shared_connection<tn_desc, tn_socket>;
        using io_context_t       = base_t::io_context_t;
        using command_t          = base_t::command_t;
        using serialize_t        = base_t::serialize_t;
        using connection_t       = client_connection<tn_desc, tn_socket>;
        using message_t          = base_t::message_t;
        using owned_message_t    = owned_message<tn_desc, connection_t>;
        using outgoing_message_t = base_t::outgoing_message_t;

    public:
        client_connection()                    = delete;
        client_connection(const connection_t&) = delete;
        client_connection(connection_t&&)      = default;

        client_connection(io_context_t& context)
            : base_t(context) {}

        connection_t& operator=(const connection_t&) = delete;
        connection_t& operator=(connection_t&&)      = default;

    public:
        virtual void stop() override {
            base_t::stop();
        }

    protected:
        virtual void notify() override {}

        virtual void direct_send(const std::shared_ptr<outgoing_message_t> outgoing_message) override {
            base_t::direct_send(outgoing_message);
        }
    };
}
