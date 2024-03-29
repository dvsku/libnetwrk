#pragma once

#include "libnetwrk/net/core/messages/message_head.hpp"
#include "libnetwrk/net/core/containers/buffer.hpp"

#include <chrono>

namespace libnetwrk {
    template<typename Command, typename Serialize>
    requires is_enum<Command>
    class message {
    public:
        using message_head_t = message_head<Command, Serialize>;
        using message_t      = message<Command, Serialize>;
        using buffer_t       = buffer<Serialize>;
        using command_t      = Command;

        using milliseconds_t = std::chrono::milliseconds;

    public:
        message_head_t head;
        buffer_t       data;
        buffer_t       data_head;

    public:
        message()                 = default;
        message(const message_t&) = default;
        message(message_t&&)      = default;

        message(command_t command) {
            head.command = command;
        }

        message_t& operator=(const message_t&) = default;
        message_t& operator=(message_t&&)      = default;

    public:
        /// <summary>
        /// Get milliseconds it took from sending the message to receiving the message.
        /// </summary>
        milliseconds_t latency() {
            return latency(head.recv_timestamp);
        }

        /// <summary>
        /// Get milliseconds it took from receiving the message to a relative timestamp.
        /// </summary>
        milliseconds_t latency(uint64_t timestamp) {
            return std::chrono::duration_cast<milliseconds_t>(milliseconds_t(head.send_timestamp) - milliseconds_t(timestamp));
        }

        template <typename T>
        message_t& operator<<(const T& value) {
            data << value;
            head.data_size = data.size();
            return *this;
        }

        template <typename T>
        message_t& operator>>(T& value) {
            data >> value;
            return *this;
        }
    };
}