#pragma once

#include "libnetwrk/net/core/messages/owned_message.hpp"
#include "libnetwrk/net/core/containers/tsdeque.hpp"

#include <string>

namespace libnetwrk {
    enum class context_owner : uint8_t {
        server = 0x0,
        client = 0x1
    };

    template<typename Tcommand, typename Tserialize, typename Tstorage>
    class context {
    public:
        using context_t       = asio::io_context;
        using owned_message_t = owned_message<Tcommand, Tserialize, Tstorage>;

    public:
        const std::string   name;
        const context_owner owner;

        std::unique_ptr<context_t> asio_context;
        tsdeque<owned_message_t>   incoming_messages;

    public:
        context(const std::string& name, context_owner owner)
            : name(name), owner(owner) {}

    };
}
