#pragma once

#include "libnetwrk/net/macros.hpp"
#include "libnetwrk/net/common/containers/buffer.hpp"
#include "libnetwrk/net/common/exceptions/libnetwrk_exception.hpp"
#include "libnetwrk/net/type_traits.hpp"

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

namespace libnetwrk {
    struct bin_serialize {
        using serializer_t = bin_serialize;
        using buffer_t     = buffer<bin_serialize>;

        ///////////////////////////////////////////////////////////////////////
        // STANDARD LAYOUT

        template<typename T>
        requires is_arithmentic_or_enum<T>
        static void serialize(buffer_t& buffer, const T& value) {
            buffer.push_back(&value, sizeof(T));
        }

        template<typename T>
        requires is_arithmentic_or_enum<T>
        static void deserialize(buffer_t& buffer, T& obj) {
            buffer.get_range(&obj, sizeof(obj));
        }

        ///////////////////////////////////////////////////////////////////////
        // SERIALIZABLE

        template <typename T>
        requires is_serializable<T, serializer_t>
        static void serialize(buffer_t& buffer, const T& obj) {
            obj.serialize(buffer);
        }

        template<typename T>
        requires is_serializable<T, serializer_t>
        static void deserialize(buffer_t& buffer, T& obj) {
            obj.deserialize(buffer);
        }

        ///////////////////////////////////////////////////////////////////////
        // STD::STRING

        static void serialize(buffer_t& buffer, const std::string& str) {
            serialize(buffer, str.size());
            buffer.push_back(str.data(), str.size());
        }

        static void deserialize(buffer_t& buffer, std::string& str) {
            size_t size = 0U;
            deserialize(buffer, size);

            // Resize string to size
            str.resize(size);

            // Get data from buffer
            buffer.get_range(str.data(), size);
        }

        ///////////////////////////////////////////////////////////////////////
        // STD::ARRAY

        template <typename T, std::size_t N>
        requires is_arithmentic_or_enum<T>
        static void serialize(buffer_t& buffer, const std::array<T, N>& container) {
            serialize(buffer, container.size());
            buffer.push_back(container.data(), container.size() * sizeof(T));
        }

        template <typename T, std::size_t N>
        static void serialize(buffer_t& buffer, const std::array<T, N>& container) {
            serialize(buffer, container.size());
            for (const T& element : container)
                serialize(buffer, element);
        }

        template <typename T, std::size_t N>
        requires is_arithmentic_or_enum<T>
        static void deserialize(buffer_t& buffer, std::array<T, N>& container) {
            size_t size = 0U;
            deserialize(buffer, size);

            if (size > N)
                throw libnetwrk_exception("out of bounds");

            buffer.get_range(container.data(), size * sizeof(T));
        }

        template <typename T, std::size_t N>
        static void deserialize(buffer_t& buffer, std::array<T, N>& container) {
            size_t size = 0U;
            deserialize(buffer, size);

            if (size > N)
                throw libnetwrk_exception("out of bounds");

            for (size_t i = 0U; i < size; i++)
                deserialize(buffer, container[i]);
        }

        ///////////////////////////////////////////////////////////////////////
        // STD::VECTOR

        template <typename T>
        requires is_arithmentic_or_enum<T>
        static void serialize(buffer_t& buffer, const std::vector<T>& container) {
            serialize(buffer, container.size());
            buffer.push_back(container.data(), container.size() * sizeof(T));
        }

        template <typename T>
        static void serialize(buffer_t& buffer, const std::vector<T>& container) {
            serialize(buffer, container.size());
            for (const T& element : container)
                serialize(buffer, element);
        }

        template <typename T>
        requires is_arithmentic_or_enum<T>
        static void deserialize(buffer_t& buffer, std::vector<T>& container) {
            size_t size = 0U;
            deserialize(buffer, size);

            container.resize(size);
            buffer.get_range(container.data(), size * sizeof(T));
        }

        template <typename T>
        static void deserialize(buffer_t& buffer, std::vector<T>& container) {
            size_t size = 0;
            deserialize(buffer, size);

            container.resize(size);
            container.clear();

            for (size_t i = 0; i < size; i++) {
                T element{};
                deserialize(buffer, element);
                container.push_back(element);
            }
        }

        ///////////////////////////////////////////////////////////////////////
        // STD::DEQUE

        template <typename T>
        static void serialize(buffer_t& buffer, const std::deque<T>& container) {
            serialize(buffer, container.size());
            for (const T& element : container)
                serialize(buffer, element);
        }

        template <typename T>
        static void deserialize(buffer_t& buffer, std::deque<T>& container) {
            size_t size = 0;
            deserialize(buffer, size);

            container = std::deque<T>();
            for (size_t i = 0; i < size; i++) {
                T element{};
                deserialize(buffer, element);
                container.push_back(element);
            }
        }

        ///////////////////////////////////////////////////////////////////////
        // STD::LIST

        template <typename T>
        static void serialize(buffer_t& buffer, const std::list<T>& container) {
            serialize(buffer, container.size());
            for (const T& element : container)
                serialize(buffer, element);
        }

        template <typename T>
        static void deserialize(buffer_t& buffer, std::list<T>& container) {
            size_t size = 0;
            deserialize(buffer, size);

            container = std::list<T>();
            for (size_t i = 0; i < size; i++) {
                T element{};
                deserialize(buffer, element);
                container.push_back(element);
            }
        }

        ///////////////////////////////////////////////////////////////////////
        // STD::FORWARD_LIST

        template <typename T>
        static void serialize(buffer_t& buffer, const std::forward_list<T>& container) {
            size_t placeholder_offset = buffer.size();
            size_t size = 0;

            // Push a placeholder
            serialize(buffer, size);

            // Serialize elements and increase size count
            for (const T& element : container) {
                serialize(buffer, element);
                size++;
            }

            // Swap placeholder with real size
            buffer.swap_at(&size, sizeof(size_t), placeholder_offset);
        }

        template <typename T>
        static void deserialize(buffer_t& buffer, std::forward_list<T>& container) {
            size_t size = 0;
            deserialize(buffer, size);

            container = std::forward_list<T>();
            for (size_t i = 0; i < size; i++) {
                T element{};
                deserialize(buffer, element);
                container.push_front(element);
            }

            container.reverse();
        }

        ///////////////////////////////////////////////////////////////////////
        // UNSUPPORTED

        SERIALIZER_UNSUPPORTED_CONTAINER(std::stack);
        SERIALIZER_UNSUPPORTED_CONTAINER(std::queue);
        SERIALIZER_UNSUPPORTED_CONTAINER(std::priority_queue);
        SERIALIZER_UNSUPPORTED_CONTAINER(std::set);
        SERIALIZER_UNSUPPORTED_CONTAINER(std::multiset);
        SERIALIZER_UNSUPPORTED_CONTAINER(std::unordered_set);
        SERIALIZER_UNSUPPORTED_CONTAINER(std::unordered_multiset);

        SERIALIZER_UNSUPPORTED_KVP_CONTAINER(std::map);
        SERIALIZER_UNSUPPORTED_KVP_CONTAINER(std::multimap);
        SERIALIZER_UNSUPPORTED_KVP_CONTAINER(std::unordered_map);
        SERIALIZER_UNSUPPORTED_KVP_CONTAINER(std::unordered_multimap);

    };
}
