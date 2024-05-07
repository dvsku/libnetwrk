#pragma once

#include "libnetwrk/net/core/messages/message.hpp"

namespace libnetwrk {
    template<typename Command, typename Serialize, typename Storage>
    class base_connection;

    template<typename Command, typename Serialize, typename Storage>
    class owned_message {
    public:
        using owned_message_t = owned_message<Command, Serialize, Storage>;
        using message_t       = message<Command, Serialize>;
        using client_t        = base_connection<Command, Serialize, Storage>;

    public:
        owned_message()                       = default;
        owned_message(const owned_message_t&) = default;
        owned_message(owned_message_t&&)      = default;

        owned_message_t& operator=(const owned_message_t&) = default;
        owned_message_t& operator=(owned_message_t&&)      = default;

    public:
        message_t                 msg;
        std::shared_ptr<client_t> sender;
    };
}