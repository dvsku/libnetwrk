#pragma once

#include "libnetwrk/net/common/messages/message_head.hpp"
#include "libnetwrk/net/common/containers/buffer.hpp"

#include <chrono>

namespace libnetwrk {
    template<typename Tcommand, typename Tserialize>
    requires is_enum<Tcommand>
    class message {
    public:
        using message_head_t = message_head<Tcommand, Tserialize>;
        using message_t      = message<Tcommand, Tserialize>;
        using buffer_t       = buffer<Tserialize>;
        using command_t      = Tcommand;

        using milliseconds_t = std::chrono::milliseconds;

    public:
        message_head_t head;
        buffer_t       data;
        buffer_t       data_head;

    public:
        message() {
            data_head.resize(head.size());
        }

        message(command_t command) {
            head.command = command;
            data_head.resize(head.size());
        }

        message(const message_t&) = default;
        message(message_t&&)      = default;

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