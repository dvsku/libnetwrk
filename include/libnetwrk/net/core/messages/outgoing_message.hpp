#pragma once

#include "libnetwrk/net/core/messages/message.hpp"
#include "libnetwrk/net/core/containers/buffer.hpp"

#include <mutex>

namespace libnetwrk {
    template<typename Desc>
    requires is_libnetwrk_service_desc<Desc>
    class outgoing_message {
    public:
        using message_t = message<Desc>;
        using buffer_t  = buffer<typename Desc::serialize_t>;

    public:
        outgoing_message()                        = delete;
        outgoing_message(const outgoing_message&) = delete;
        outgoing_message(outgoing_message&&)      = delete;

        outgoing_message(const message_t& message)
            : message(message) {}

        outgoing_message(message_t&& message)
            : message(std::move(message)) {}

        outgoing_message& operator=(const outgoing_message&) = delete;
        outgoing_message& operator=(outgoing_message&&)      = delete;

    public:
        message_t  message;
        buffer_t   serialized_head;
        std::mutex mutex;
    };
}
