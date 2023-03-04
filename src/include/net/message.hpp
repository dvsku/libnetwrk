#ifndef LIBNETWRK_NET_COMMON_MESSAGE_H
#define LIBNETWRK_NET_COMMON_MESSAGE_H

#include <type_traits>
#include <memory>

#include "net/common/serialization/type_traits.hpp"
#include "net/common/serialization/serialize.hpp"
#include "net/definitions.hpp"
#include "utilities/buffers.hpp"

namespace libnetwrk::net {
    template <typename T> struct message_head {
        T m_command{};
        uint32_t m_data_len;

        message_head() : m_data_len(0) {}
    };
    
    template <typename T> 
    class message {
        public:
            message_head<T> m_head;
            BUFFER_U8 m_data;

            message() {
                m_data.resize(0);
            }

            message(T command) {
                m_head.m_command = command;
                m_data.resize(0);
            }

            // Returns the size of the message (head + data)
            size_t size_of() const {
                return sizeof(message_head<T>) + m_data.size();
            }

            ///////////////////////////////////////////////////////////////////
            //  SERIALIZE
            ///////////////////////////////////////////////////////////////////

            // Serializes a primitive type
            template <typename DataType>
            friend message<T>& operator << (message<T>& msg, const DataType& data) {
                push(msg, data);
                return msg;
            }

            // Serializes a vector of primitive types
            template <typename DataType>
            friend message<T>& operator << (message<T>& msg, const BUFFER_<DataType>& data) {
                push(msg, data);
                return msg;
            }

            // Serializes a string
            friend message<T>& operator << (message<T>& msg, const std::string& data) {
                push(msg, data);
                return msg;
            }

            // Serializes a vector of strings
            friend message<T>& operator << (message<T>& msg, const BUFFER_STR& data) {
                push(msg, data);
                return msg;
            }

            ///////////////////////////////////////////////////////////////////
            //  DESERIALIZE
            ///////////////////////////////////////////////////////////////////

            // Deserializes a primitive type
            template <typename DataType>
            friend message<T>& operator >> (message<T>& msg, DataType& data) {
                pop(msg, data);
                return msg;
            }

            // Deserializes a vector of primitive types
            template <typename DataType>
            friend message<T>& operator >> (message<T>& msg, BUFFER_<DataType>& data) {
                pop(msg, data);
                return msg;
            }

            // Deserializes a string
            friend message<T>& operator >> (message<T>& msg, std::string& data) {
                pop(msg, data);
                return msg;
            }

            // Deserializes a vector of strings
            friend message<T>& operator >> (message<T>& msg, BUFFER_STR& data) {
                pop(msg, data);
                return msg;
            }

            ///////////////////////////////////////////////////////////////////
            // PUSH/POP
            ///////////////////////////////////////////////////////////////////

        protected:
            static void push(message<T>& msg, const std::string& data) {
                BUFFER_U8 bytes = libnetwrk::net::common::serialize(data);
                msg.push_bytes(bytes);
            }

            static void push(message<T>& msg, const BUFFER_STR& data) {
                BUFFER_U8 bytes = libnetwrk::net::common::serialize(data);
                msg.push_bytes(bytes);
            }

            template <typename DataType>
            static void push(message<T>& msg, const DataType& data, 
                typename std::enable_if<std::is_standard_layout<DataType>::value, bool>::type = true)
            {
                BUFFER_U8 bytes = libnetwrk::net::common::serialize(data);
                msg.push_bytes(bytes);
            }

            template <typename DataType>
            static void push(message<T>& msg, const DataType& data, 
                typename std::enable_if<libnetwrk::net::common::is_serializable<DataType>::value, bool>::type = true)
            {
                BUFFER_U8 bytes = data.serialize();
                msg.push_bytes(bytes);
            }

            template <typename DataType>
            static void push(message<T>& msg, const BUFFER_<DataType>& data, 
                typename std::enable_if<std::is_standard_layout<DataType>::value, bool>::type = true)
            {
                BUFFER_U8 bytes = libnetwrk::net::common::serialize(data);
                msg.push_bytes(bytes);
            }

            template <typename DataType>
            static void push(message<T>& msg, const BUFFER_<DataType>& data, 
                typename std::enable_if<libnetwrk::net::common::is_serializable<DataType>::value, bool>::type = true)
            {
                BUFFER_U8 bytes;

                size_t num_el = data.size();

                // BECAUSE DERIVED STRUCT SIZE IS UNKNOWN IF THERE ARE NO ELEMENTS
                // RETURN JUST THE ELEMENT COUNT (WHICH IS 0)
                if (data.size() == 0) {
                    bytes.resize(sizeof(size_t));
                    std::memcpy(&bytes[0], &num_el, sizeof(size_t));
                    msg.push_bytes(bytes);
                    return;
                }

                size_t offset = 0;

                bytes.resize(data.size() * data[0].size() + sizeof(size_t));

                for (int i = num_el - 1; i >= 0; i--) {
                    BUFFER_U8 serialized = data[i].serialize();
                    std::memcpy(&bytes[0] + offset, &serialized[0], serialized.size());
                    offset += serialized.size();
                }

                std::memcpy(&bytes[0] + offset, &num_el, sizeof(size_t));

                msg.push_bytes(bytes);
            }

            template <typename DataType>
            static void pop(message<T>& msg, DataType& data, 
                typename std::enable_if<std::is_standard_layout<DataType>::value, bool>::type = true)
            {
                size_t offset = msg.m_data.size() - sizeof(DataType);
                std::memcpy(&data, &msg.m_data[0] + offset, sizeof(DataType));
                msg.m_data.resize(offset);
                msg.m_head.m_data_len = msg.m_data.size();
            }

            template <typename DataType>
            static void pop(message<T>& msg, DataType& data, 
                typename std::enable_if<libnetwrk::net::common::is_serializable<DataType>::value, bool>::type = true)
            {
                size_t offset = msg.m_data.size() - data.size();
                data.deserialize(libnetwrk::utilities::get_range(msg.m_data, offset, data.size()));
                msg.m_data.resize(offset);
                msg.m_head.m_data_len = msg.m_data.size();
            }

            template <typename DataType>
            static void pop(message<T>& msg, BUFFER_<DataType>& data, 
                typename std::enable_if<std::is_standard_layout<DataType>::value, bool>::type = true)
            {
                size_t count = 0;
                msg >> count;

                for (size_t i = 0; i < count; i++) {
                    DataType value;
                    msg >> value;
                    data.push_back(value);
                }
            }

            template <typename DataType>
            static void pop(message<T>& msg, BUFFER_<DataType>& data, 
                typename std::enable_if<libnetwrk::net::common::is_serializable<DataType>::value, bool>::type = true)
            {
                size_t count = 0;
                msg >> count;

                DataType temp;
                size_t size = temp.size();

                for (size_t i = 0; i < count; i++) {
                    BUFFER_U8 bytes = libnetwrk::utilities::get_range(msg.m_data, msg.m_data.size() - size, size);
                    temp.deserialize(bytes);
                    data.push_back(temp);
                    msg.m_data.resize(msg.m_data.size() - temp.size());
                }
            }

            static void pop(message<T>& msg, std::string& data) {
                size_t string_size = 0;
                msg >> string_size;

                size_t offset = msg.m_data.size() - string_size;

                char* c_str = new char[string_size];
                std::memcpy(c_str, &msg.m_data[0] + offset, string_size);

                data = std::string(c_str);

                msg.m_data.resize(offset);
                msg.m_head.m_data_len = msg.m_data.size();

                delete[] c_str;
            }

            static void pop(message<T>& msg, BUFFER_STR& data) {
                size_t count = 0;
                msg >> count;

                for (size_t i = 0; i < count; i++) {
                    std::string value;
                    msg >> value;
                    data.push_back(value);
                }
            }

        private:
            void push_bytes(BUFFER_U8& bytes) {
                size_t offset = m_data.size();
                size_t size = bytes.size();

                m_data.resize(m_data.size() + size);
                std::memcpy(&m_data[0] + offset, &bytes[0], size);

                m_head.m_data_len = m_data.size();
            }
    };

    // FORWARD DECLARE
    namespace common {
        struct nothing;
        template <typename A, typename B> class base_connection;
    }

    template <typename command_type, typename storage = common::nothing>
    class owned_message {
        public:
            message<command_type> m_msg;
            std::shared_ptr<common::base_connection<command_type, storage>> m_client;
    };
}

#endif