#pragma once

#include "libnetwrk/net/common/messages/message.hpp"

namespace libnetwrk {
    template<typename Tcommand, typename Tserialize, typename Tstorage>
    class base_connection;

    template<typename Tcommand, typename Tserialize, typename Tstorage>
    class owned_message {
    public:
        using owned_message_t = owned_message<Tcommand, Tserialize, Tstorage>;
        using message_t       = message<Tcommand, Tserialize>;
        using client_t        = base_connection<Tcommand, Tserialize, Tstorage>;

    public:
        owned_message()                       = default;
        owned_message(const owned_message_t&) = default;
        owned_message(owned_message_t&&)      = default;

        owned_message_t& operator=(const owned_message_t&) = default;
        owned_message_t& operator=(owned_message_t&&)      = default;

    public:
        message_t                 message;
        std::shared_ptr<client_t> client;
    };
}