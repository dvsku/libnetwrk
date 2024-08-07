#pragma once

#include "libnetwrk/net/messages/message_head.hpp"
#include "libnetwrk/net/containers/dynamic_buffer.hpp"

#include <chrono>

namespace libnetwrk {
    template<typename Desc>
    requires libnetwrk_desc<Desc>
    class message {
    public:
        using message_head_t = message_head<Desc>;
        using message_t      = message<Desc>;
        using command_t      = typename Desc::command_t;

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
            Get end-to-end latency.
        */
        std::chrono::milliseconds latency() {
            return latency(head.recv_timestamp);
        }

        /*
            Get send timestamp to relative timestamp latency.
            Relative timestamp is assumed to be in ms.
        */
        std::chrono::milliseconds latency(uint64_t timestamp) {
            return std::chrono::milliseconds(std::max((int64_t)0, (int64_t)(timestamp - head.send_timestamp)));
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