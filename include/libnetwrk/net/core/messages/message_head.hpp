#pragma once

#include "libnetwrk/net/type_traits.hpp"

namespace libnetwrk {
    enum class message_type : uint8_t {
        system = 0x0,
        user   = 0x1
    };

    template<typename Desc>
    requires is_libnetwrk_service_desc<Desc>
    class message_head {
    public:
        using message_head_t = message_head<Desc>;
        using message_type_t = message_type;
        using buffer_t       = buffer<typename Desc::serialize_t>;
        using command_t      = typename Desc::command_t;

    public:
        message_type_t type           = message_type_t::user;
        uint64_t       command        = 0U;
        uint64_t       send_timestamp = 0U;
        uint64_t       recv_timestamp = 0U;
        size_t         data_size      = 0U;

    public:
        message_head()                    = default;
        message_head(const message_head&) = default;

        message_head(message_head&& head) noexcept {
            this->type           = head.type;
            this->command        = head.command;
            this->send_timestamp = head.send_timestamp;
            this->recv_timestamp = head.recv_timestamp;
            this->data_size      = head.data_size;

            head.type           = message_type_t{};
            head.command        = 0U;
            head.send_timestamp = 0U;
            head.recv_timestamp = 0U;
            head.data_size      = 0U;
        }

        message_head_t& operator=(const message_head_t&) = default;

        message_head_t& operator=(message_head_t&& head) noexcept {
            if (this != &head) {
                this->type           = head.type;
                this->command        = head.command;
                this->send_timestamp = head.send_timestamp;
                this->recv_timestamp = head.recv_timestamp;
                this->data_size      = head.data_size;

                head.type           = message_type_t{};
                head.command        = 0U;
                head.send_timestamp = 0U;
                head.recv_timestamp = 0U;
                head.data_size      = 0U;
            }
            return *this;
        }

    public:
        constexpr size_t size() const {
            return sizeof(type) + sizeof(command) + sizeof(send_timestamp) + sizeof(data_size);
        }

        template<typename T>
        void serialize(buffer<T>& buffer) const {
            buffer << type << command << send_timestamp << data_size;
        }

        template<typename T>
        void deserialize(buffer<T>& buffer) {
            buffer >> type >> command >> send_timestamp >> data_size;
        }
    };
}