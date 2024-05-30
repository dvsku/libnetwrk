#pragma once

#include "libnetwrk/net/messages/message_head.hpp"
#include "libnetwrk/net/containers/dynamic_buffer.hpp"

#include <chrono>

namespace libnetwrk {
    template<typename Desc>
    requires is_libnetwrk_service_desc<Desc>
    class message {
    public:
        using message_head_t = message_head<Desc>;
        using message_t      = message<Desc>;
        using command_t      = typename Desc::command_t;

        using milliseconds_t = std::chrono::milliseconds;

    public:
        message_head_t head;
        dynamic_buffer data;

    public:
        message()                 = default;
        message(const message_t&) = default;
        message(message_t&&)      = default;

        message(command_t command) {
            set_command(command);
        }

        message_t& operator=(const message_t&) = default;
        message_t& operator=(message_t&&)      = default;

    public:
        command_t command() {
            return static_cast<command_t>(head.command);
        }

        void set_command(command_t command) {
            head.command = static_cast<uint64_t>(command);
        }

        /*
            Get milliseconds it took from sending the message to receiving the message.
        */
        milliseconds_t latency() {
            return latency(head.recv_timestamp);
        }

        /*
            Get milliseconds it took from receiving the message to a relative timestamp.
        */
        milliseconds_t latency(uint64_t timestamp) {
            return std::chrono::duration_cast<milliseconds_t>(milliseconds_t(head.send_timestamp) - milliseconds_t(timestamp));
        }

        template <typename T>
        message_t& operator<<(const T& value) {
            data << value;
            head.data_size = (uint32_t)data.size();
            return *this;
        }

        template <typename T>
        message_t& operator>>(T& value) {
            data >> value;
            return *this;
        }
    };
}