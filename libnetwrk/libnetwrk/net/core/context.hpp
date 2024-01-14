#pragma once

#include "libnetwrk/net/core/messages/owned_message.hpp"
#include "libnetwrk/net/core/containers/tsdeque.hpp"

#include <string>

namespace libnetwrk {
    enum class context_owner : uint8_t {
        server = 0x0,
        client = 0x1
    };

    template<typename Command, typename Serialize, typename Storage>
    class context {
    public:
        using context_t         = asio::io_context;
        using owned_message_t   = owned_message<Command, Serialize, Storage>;
        using base_connection_t = base_connection<Command, Serialize, Storage>;

    public:
        const std::string   name;
        const context_owner owner;

        std::unique_ptr<context_t> asio_context;
        tsdeque<owned_message_t>   incoming_messages;

    public:
        context(const std::string& name, context_owner owner)
            : name(name), owner(owner) {}

    public:
        // Called when processing messages
        virtual void ev_message(owned_message_t& msg) = 0;

        virtual void internal_ev_client_disconnected(std::shared_ptr<base_connection_t> client) = 0;
    };
}
