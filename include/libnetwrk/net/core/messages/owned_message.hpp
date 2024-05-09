#pragma once

#include "libnetwrk/net/core/messages/message.hpp"

namespace libnetwrk {
    template<typename Desc, typename Connection>
    requires is_libnetwrk_service_desc<Desc>
    class owned_message {
    public:
        using owned_message_t = owned_message<Desc, Connection>;
        using message_t       = message<Desc>;
        using connection_t    = Connection;

    public:
        owned_message()                       = default;
        owned_message(const owned_message_t&) = default;
        owned_message(owned_message_t&&)      = default;

        owned_message_t& operator=(const owned_message_t&) = default;
        owned_message_t& operator=(owned_message_t&&)      = default;

    public:
        message_t                     msg;
        std::shared_ptr<connection_t> sender;
    };
}