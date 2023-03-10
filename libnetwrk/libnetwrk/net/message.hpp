#ifndef LIBNETWRK_NET_MESSAGE_HPP
#define LIBNETWRK_NET_MESSAGE_HPP

#include <type_traits>

#include "libnetwrk/net/common/serialization/serializers/binary_serializer.hpp"
#include "libnetwrk/net/common/containers/buffer.hpp"
#include "libnetwrk/net/common/serialization/type_traits.hpp"
#include "libnetwrk/net/definitions.hpp"

namespace libnetwrk::net {
    template <typename T> struct message_head {
        T m_command{};
        uint32_t m_data_len;

        message_head() : m_data_len(0) {}
    };
    
    template <typename T, typename serializer = libnetwrk::net::common::binary_serializer>
    class message {
        public:
            typedef message_head<T> message_head_t;
            typedef libnetwrk::net::common::buffer<serializer> buffer_t;
            typedef message<T, serializer> message_t;

            message_head_t m_head;
            buffer_t m_data;

            message() {}

            message(T command) {
                m_head.m_command = command;
            }

            // Returns the size of the message (head + data)
            size_t size_of() const {
                return sizeof(message_head_t) + m_data.size();
            }

            ///////////////////////////////////////////////////////////////////
            //  SERIALIZE
            ///////////////////////////////////////////////////////////////////

            template <typename TObj>
            friend message_t& operator << (message_t& msg, const TObj& value) {
                msg.m_data << value;
                msg.m_head.m_data_len = msg.m_data.size();
                return msg;
            }

            ///////////////////////////////////////////////////////////////////
            //  DESERIALIZE
            ///////////////////////////////////////////////////////////////////

            template <typename TObj>
            friend message_t& operator >> (message_t& msg, TObj& obj) {
                msg.m_data >> obj;
                return msg;
            }
    };

    namespace common {
        template <typename command_type, typename serializer, typename storage>
        class base_connection;
    }

    template <typename command_type,
        typename serializer = libnetwrk::net::common::binary_serializer,
        typename storage = libnetwrk::nothing>
    class owned_message {
        public:
            typedef message<command_type, serializer> message_t;
            typedef libnetwrk::net::common::base_connection<command_type, serializer, storage> base_connection_t;
            typedef std::shared_ptr<base_connection_t> base_connection_t_ptr;

            message_t m_msg;
            base_connection_t_ptr m_client;
    };
}

#endif