#pragma once

#include "libnetwrk/net/messages/message.hpp"
#include "libnetwrk/net/containers/fixed_buffer.hpp"

#include <mutex>

namespace libnetwrk {
    template<typename Desc>
    requires is_libnetwrk_service_desc<Desc>
    class outgoing_message {
    public:
        using message_t = libnetwrk::message<Desc>;

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
        message_t                                     message;
        fixed_buffer<message_t::message_head_t::size> serialized_head;
        std::mutex                                    mutex;
    };
}
