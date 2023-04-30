#ifndef LIBNETWRK_NET_MESSAGE_HPP
#define LIBNETWRK_NET_MESSAGE_HPP

#include <type_traits>
#include <chrono>

#include "libnetwrk/net/common/serialization/serializers/binary_serializer.hpp"
#include "libnetwrk/net/common/containers/buffer.hpp"
#include "libnetwrk/net/type_traits.hpp"
#include "libnetwrk/net/definitions.hpp"
#include "libnetwrk/net/macros.hpp"

namespace libnetwrk::net {
    template <typename command_type,
        typename serializer = libnetwrk::net::common::binary_serializer>
    struct message_head : public libnetwrk::net::common::serializable<serializer> {
        typedef message_head<command_type, serializer> message_head_t;
        typedef libnetwrk::net::common::buffer<serializer> buffer_t;

        command_type m_command{};
        uint32_t m_data_len = 0;
        uint64_t m_send_timestamp = 0;
        uint64_t m_receive_timestamp = 0;

        message_head() {}

        message_head(const message_head_t& head) = default;
        message_head_t& operator= (const message_head_t& head) = default;

        message_head(message_head_t&& head) noexcept {
            m_command = head.m_command;
            m_data_len = head.m_data_len;
            m_send_timestamp = head.m_send_timestamp;
            m_receive_timestamp = head.m_receive_timestamp;

            head.m_command = command_type{};
            head.m_data_len = 0;
            head.m_send_timestamp = 0;
            head.m_receive_timestamp = 0;
        };

        message_head_t& operator= (message_head_t&& head) noexcept {
            if (this != &head) {
                m_command = head.m_command;
                m_data_len = head.m_data_len;
                m_send_timestamp = head.m_send_timestamp;
                m_receive_timestamp = head.m_receive_timestamp;

                head.m_command = command_type{};
                head.m_data_len = 0;
                head.m_send_timestamp = 0;
                head.m_receive_timestamp = 0;
            }
            return *this;
        };
        
        ///////////////////////////////////////////////////////////////////////
        // Serializable
        ///////////////////////////////////////////////////////////////////////

        buffer_t serialize() const override {
            buffer_t buffer;
            buffer << m_command << m_send_timestamp << m_data_len;
            return buffer;
        }

        void deserialize(buffer_t serialized) override {
            serialized >> m_command >> m_send_timestamp >> m_data_len;
        }
    };
    
    template <typename command_type, 
        typename serializer = libnetwrk::net::common::binary_serializer>
    class message {
        public:
            typedef message_head<command_type> message_head_t;
            typedef libnetwrk::net::common::buffer<serializer> buffer_t;
            typedef message<command_type, serializer> message_t;

            message_head_t m_head;
            buffer_t m_head_data, m_data;

            message() {
                LIBNETWRK_STATIC_ASSERT_OR_THROW(std::is_enum<command_type>::value,
                    "message command_type template arg can only be an enum");

                m_head_data.resize(m_head.serialize().size());
            }

            message(command_type command) {
                LIBNETWRK_STATIC_ASSERT_OR_THROW(std::is_enum<command_type>::value,
                    "message command_type template arg can only be an enum");

                m_head.m_command = command;
                m_head_data.resize(m_head.serialize().size());
            }

            message(const message_t& msg) = default;
            message(message_t&& msg) = default;

            message_t& operator= (const message_t& msg) = default;
            message_t& operator= (message_t&& msg) = default;

            std::chrono::milliseconds latency() {
                return std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::milliseconds(m_head.m_send_timestamp) - std::chrono::milliseconds(m_head.m_receive_timestamp));
            }

            ///////////////////////////////////////////////////////////////////
            //  SERIALIZE
            ///////////////////////////////////////////////////////////////////

            /// <summary>
            /// Add object to message
            /// </summary>
            template <typename TObj>
            friend message_t& operator << (message_t& msg, const TObj& value) {
                msg.m_data << value;
                msg.m_head.m_data_len = msg.m_data.size();
                return msg;
            }

            ///////////////////////////////////////////////////////////////////
            //  DESERIALIZE
            ///////////////////////////////////////////////////////////////////

            /// <summary>
            /// Remove object from message
            /// </summary>
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
            typedef base_connection_t_ptr client_ptr;

            message_t m_msg;
            client_ptr m_client;
    };
}

#endif