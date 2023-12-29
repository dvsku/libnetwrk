#ifndef LIBNETWRK_NET_COMMON_BINARY_SERIALIZER_HPP
#define LIBNETWRK_NET_COMMON_BINARY_SERIALIZER_HPP

#include <vector>
#include <deque>
#include <forward_list>
#include <list>
#include <stack>
#include <queue>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

#include "libnetwrk/net/macros.hpp"
#include "libnetwrk/net/common/containers/buffer.hpp"
#include "libnetwrk/net/common/exceptions/libnetwrk_exception.hpp"
#include "libnetwrk/net/type_traits.hpp"

namespace libnetwrk::net::common {
    struct binary_serializer {
        using serializer_t = binary_serializer;
        using buffer_t     = buffer<binary_serializer>;

        ///////////////////////////////////////////////////////////////////////
        // Standard layout
        ///////////////////////////////////////////////////////////////////////

        template<typename T>
        requires is_arithmentic_or_enum<T>
        static void serialize(buffer_t& buffer, const T& value) {
            buffer.push_back(&value, sizeof(T));
        }

        template<typename T>
        requires is_arithmentic_or_enum<T>
        static void serialize(buffer_t& buffer, const T& value, const size_t offset) {
            buffer.push_at(&value, sizeof(T), offset);
        }

        template<typename T>
        requires is_arithmentic_or_enum<T>
        static void deserialize(buffer_t& buffer, T& obj) {
            buffer.get_range(&obj, sizeof(obj));
        }

        ///////////////////////////////////////////////////////////////////////
        // Serializable
        ///////////////////////////////////////////////////////////////////////

        template <typename T>
        requires is_serializable<T, serializer_t>
        static void serialize(buffer_t& buffer, const T& obj) {
            buffer_t serialized;
            obj.serialize(serialized);
            serialize(buffer, serialized.size());
            buffer.push_back(serialized);
        }

        template<typename T>
        requires is_serializable<T, serializer_t>
        static void serialize(buffer_t& buffer, const T& obj, const size_t offset) {
            buffer_t serialized = obj.serialize();
            size_t size = serialized.size();
            buffer.push_at(&size, sizeof(size_t), offset);
            buffer.push_at(serialized, offset + sizeof(size_t));
        }

        template<typename T>
        requires is_serializable<T, serializer_t>
        static void deserialize(buffer_t& buffer, T& obj) {
            size_t size = 0;
            deserialize(buffer, size);
            auto range = buffer.get_range(size);
            obj.deserialize(range);
        }

        ///////////////////////////////////////////////////////////////////////
        // std::string
        ///////////////////////////////////////////////////////////////////////

        static void serialize(buffer_t& buffer, const std::string& str) {
            serialize(buffer, str.size());
            for (const char& element : str)
                serialize(buffer, element);
        }

        static void deserialize(buffer_t& buffer, std::string& str) {
            size_t size = 0;

            // Get # of elements
            deserialize(buffer, size);

            str = std::string();
            for (size_t i = 0; i < size; i++) {
                char element = 0;
                deserialize(buffer, element);
                str.push_back(element);
            }
        }

        ///////////////////////////////////////////////////////////////////////
        // std::array
        ///////////////////////////////////////////////////////////////////////

        template <typename T, std::size_t N>
        static void serialize(buffer_t& buffer, const std::array<T, N>& container) {
            serialize(buffer, container.size());
            for (const T& element : container)
                serialize(buffer, element);
        }

        template <typename T, std::size_t N>
        static void deserialize(buffer_t& buffer, std::array<T, N>& container) {
            size_t size = 0;

            // Get # of elements
            deserialize(buffer, size);

            if (size > N)
                throw libnetwrk::net::common::libnetwrk_exception();

            for (size_t i = 0; i < size; i++) {
                T element{};
                deserialize(buffer, element);
                container[i] = element;
            }
        }

        ///////////////////////////////////////////////////////////////////////
        // std::vector
        ///////////////////////////////////////////////////////////////////////

        SERIALIZER_SUPPORTED_SERIALIZE_SINGLE(std::vector) {
            serialize(buffer, container.size());
            for (const TValue& element : container)
                serialize(buffer, element);
        }

        SERIALIZER_SUPPORTED_DESERIALIZE_SINGLE(std::vector) {
            size_t size = 0;
            deserialize(buffer, size);

            container = std::vector<TValue>();
            for (size_t i = 0; i < size; i++) {
                TValue element{};
                deserialize(buffer, element);
                container.push_back(element);
            }
        }

        ///////////////////////////////////////////////////////////////////////
        // std::deque
        ///////////////////////////////////////////////////////////////////////

        SERIALIZER_SUPPORTED_SERIALIZE_SINGLE(std::deque) {
            serialize(buffer, container.size());
            for (const TValue& element : container)
                serialize(buffer, element);
        }

        SERIALIZER_SUPPORTED_DESERIALIZE_SINGLE(std::deque) {
            size_t size = 0;
            deserialize(buffer, size);

            container = std::deque<TValue>();
            for (size_t i = 0; i < size; i++) {
                TValue element{};
                deserialize(buffer, element);
                container.push_back(element);
            }
        }

        ///////////////////////////////////////////////////////////////////////
        // std::list
        ///////////////////////////////////////////////////////////////////////

        SERIALIZER_SUPPORTED_SERIALIZE_SINGLE(std::list) {
            serialize(buffer, container.size());
            for (const TValue& element : container)
                serialize(buffer, element);
        }

        SERIALIZER_SUPPORTED_DESERIALIZE_SINGLE(std::list) {
            size_t size = 0;
            deserialize(buffer, size);

            container = std::list<TValue>();
            for (size_t i = 0; i < size; i++) {
                TValue element{};
                deserialize(buffer, element);
                container.push_back(element);
            }
        }

        ///////////////////////////////////////////////////////////////////////
        // std::forward_list
        ///////////////////////////////////////////////////////////////////////

        SERIALIZER_SUPPORTED_SERIALIZE_SINGLE(std::forward_list) {
            size_t size = 0, offset = buffer.size();

            // Serialize elements and increase size count
            for (const TValue& element : container) {
                serialize(buffer, element);
                size++;
            }

            // After we know the size, serialize it before elements
            serialize(buffer, size, offset);
        }

        SERIALIZER_SUPPORTED_DESERIALIZE_SINGLE(std::forward_list) {
            size_t size = 0;
            deserialize(buffer, size);

            container = std::forward_list<TValue>();
            for (size_t i = 0; i < size; i++) {
                TValue element{};
                deserialize(buffer, element);
                container.push_front(element);
            }

            container.reverse();
        }

        ///////////////////////////////////////////////////////////////////////
        // Unsupported types
        ///////////////////////////////////////////////////////////////////////

        SERIALIZER_UNSUPPORTED_SINGLE(std::stack);
        SERIALIZER_UNSUPPORTED_SINGLE(std::queue);
        SERIALIZER_UNSUPPORTED_SINGLE(std::priority_queue);
        SERIALIZER_UNSUPPORTED_SINGLE(std::set);
        SERIALIZER_UNSUPPORTED_SINGLE(std::multiset);
        SERIALIZER_UNSUPPORTED_SINGLE(std::unordered_set);
        SERIALIZER_UNSUPPORTED_SINGLE(std::unordered_multiset);

        SERIALIZER_UNSUPPORTED_PAIR(std::map);
        SERIALIZER_UNSUPPORTED_PAIR(std::multimap);
        SERIALIZER_UNSUPPORTED_PAIR(std::unordered_map);
        SERIALIZER_UNSUPPORTED_PAIR(std::unordered_multimap);

    };
}

#endif