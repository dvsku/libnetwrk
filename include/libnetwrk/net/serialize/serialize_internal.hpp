#pragma once

#include "libnetwrk/net/containers/buffer.hpp"
#include "libnetwrk/net/serialize/serialize_adapters.hpp"
#include "libnetwrk/net/type_traits.hpp"

#include <cstdint>
#include <string>
#include <array>
#include <deque>
#include <list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

namespace {
    template <typename...>
    inline constexpr bool assert_force_false = false;

    ////////////////////////////////////////////////////////////////////////////
    // SUPPORTED

    template<typename T>
    struct marked_supported : std::false_type {};

    template<typename T, size_t N>
    struct marked_supported<std::array<T, N>> : std::true_type {};

    template<typename... Args>
    struct marked_supported<std::vector<Args...>> : std::true_type {};

    template<typename... Args>
    struct marked_supported<std::deque<Args...>> : std::true_type {};

    template<typename... Args>
    struct marked_supported<std::list<Args...>> : std::true_type {};

    template<typename... Args>
    struct marked_supported<std::set<Args...>> : std::true_type {};

    template<typename... Args>
    struct marked_supported<std::unordered_set<Args...>> : std::true_type {};

    template<typename... Args>
    struct marked_supported<std::map<Args...>> : std::true_type {};

    template<typename... Args>
    struct marked_supported<std::unordered_map<Args...>> : std::true_type {};

    template<>
    struct marked_supported<std::string> : std::true_type {};

    ////////////////////////////////////////////////////////////////////////////
    // UNSUPPORTED

    template<typename Type>
    struct marked_unsupported : std::false_type {};

    template<typename... Args>
    struct marked_unsupported<std::vector<bool, Args...>> : std::true_type {};

    template<typename Type>
    concept marked_supported_vector = !std::same_as<Type, bool>;

    ////////////////////////////////////////////////////////////////////////////
    // PRIMITIVE

    template<typename Type>
    concept is_primitive = std::same_as<Type, bool>     ||
                           std::same_as<Type, char>     ||
                           std::same_as<Type, int8_t>   ||
                           std::same_as<Type, uint8_t>  ||
                           std::same_as<Type, int16_t>  ||
                           std::same_as<Type, uint16_t> ||
                           std::same_as<Type, int32_t>  ||
                           std::same_as<Type, uint32_t> ||
                           std::same_as<Type, int64_t>  ||
                           std::same_as<Type, uint64_t> ||
                           std::same_as<Type, float>    ||
                           std::same_as<Type, double>;

    ////////////////////////////////////////////////////////////////////////////
    // SERIALIZABLE

    template<typename From, typename To>
    concept is_convertible_to = std::is_convertible_v<From, To> && requires { static_cast<To>(std::declval<From>()); };

    template<typename Buffer, typename Type>
    concept has_serialize_func = requires(const Type value, Buffer& buffer) {
        { value.serialize(buffer) } -> is_convertible_to<void>;
    };

    template<typename Buffer, typename Type>
    concept has_deserialize_func = requires(Type value, Buffer& buffer) {
        { value.deserialize(buffer) } -> is_convertible_to<void>;
        requires (!std::is_const_v<Type>);
    };
}

namespace libnetwrk::serialize::internal {

    template<typename Buffer>
    concept is_supported_buffer = libnetwrk::is_base_of<libnetwrk::buffer, Buffer>;

    template<typename Buffer, typename Type>
    concept is_serializable = is_supported_buffer<Buffer> && has_serialize_func<Buffer, Type>&&
        has_deserialize_func<Buffer, Type>;

    template<typename Buffer, typename Type>
    concept is_supported = (marked_supported<Type>::value && !marked_unsupported<Type>::value) || 
        is_primitive<Type> || is_serializable<Buffer, Type>;

    template<typename Buffer, typename Type>
    concept is_unsupported = !is_supported<Buffer, Type>;

    template<typename Buffer, typename Type>
    requires is_unsupported<Buffer, Type>
    void serialize(Buffer& buffer, const Type& value) {
        static_assert(assert_force_false<Type>, "Called serialize() or << on a buffer with an unsupported type as the arg.");
    }

    template<typename Buffer, typename Type>
    requires is_unsupported<Buffer, Type>
    void deserialize(Buffer& buffer, Type& value) {
        static_assert(assert_force_false<Type>, "Called deserialize() or >> on a buffer with an unsupported type as the arg.");
    }

    ////////////////////////////////////////////////////////////////////////////
    // SERIALIZE

    template<typename Buffer, typename Type>
    requires is_primitive<Type>
    void serialize(Buffer& buffer, const Type& value) {
        internal::write(buffer, static_cast<const uint8_t*>(static_cast<const void*>(&value)), sizeof(Type));
    }

    template<typename Buffer, typename Type>
    requires is_serializable<Buffer, Type>
    void serialize(Buffer& buffer, const Type& value) {
        value.serialize(buffer);
    }

    template<typename Buffer>
    uint32_t serialize(Buffer& buffer, const std::size_t& value) {
        if constexpr (sizeof(std::size_t) == 8) {
            uint32_t casted = static_cast<uint32_t>(value);
            serialize(buffer, casted);
            return casted;
        }
        else {
            internal::write(buffer, static_cast<const uint8_t*>(static_cast<const void*>(&value)), sizeof(std::size_t));
            return value;
        }
    }

    template<typename Buffer>
    void serialize(Buffer& buffer, const std::string& value) {
        uint32_t size = serialize(buffer, value.size());

        if (size != value.size())
            throw libnetwrk_exception("serialize: std::string size truncated.");

        internal::write(buffer, static_cast<const uint8_t*>(static_cast<const void*>(value.data())), size);
    }

    template<typename Buffer, typename Type, size_t Size>
    void serialize(Buffer& buffer, const std::array<Type, Size>& value) {
        uint32_t size = serialize(buffer, value.size());

        if (size != value.size())
            throw libnetwrk_exception("serialize: std::array size truncated.");

        if constexpr (is_primitive<Type>) {
            internal::write(buffer, 
                static_cast<const uint8_t*>(static_cast<const void*>(value.data())), size * sizeof(Type));
        }
        else {
            for (uint32_t i = 0; i < size; i++) {
                serialize(buffer, value[i]);
            }
        }
    }

    template<typename Buffer, typename Type>
    requires marked_supported_vector<Type>
    void serialize(Buffer& buffer, const std::vector<Type>& value) {
        uint32_t size = serialize(buffer, value.size());

        if (size != value.size())
            throw libnetwrk_exception("serialize: std::vector size truncated.");

        if constexpr (is_primitive<Type>) {
            internal::write(buffer,
                static_cast<const uint8_t*>(static_cast<const void*>(value.data())), size * sizeof(Type));
        }
        else {
            for (uint32_t i = 0; i < size; i++) {
                serialize(buffer, value[i]);
            }
        }
    }

    template<typename Buffer, typename Type>
    void serialize(Buffer& buffer, const std::deque<Type>& value) {
        uint32_t size = serialize(buffer, value.size());

        if (size != value.size())
            throw libnetwrk_exception("serialize: std::deque size truncated.");

        for (uint32_t i = 0; i < size; i++) {
            serialize(buffer, value[i]);
        }
    }

    template<typename Buffer, typename Type>
    void serialize(Buffer& buffer, const std::list<Type>& value) {
        uint32_t size = serialize(buffer, value.size());

        if (size != value.size())
            throw libnetwrk_exception("serialize: std::list size truncated.");

        uint32_t count = 0;
        for (auto& element : value) {
            if (count == size) break;

            serialize(buffer, element);
        }
    }

    template<typename Buffer, typename Type, typename... Args>
    void serialize(Buffer& buffer, const std::set<Type, Args...>& value) {
        uint32_t size = serialize(buffer, value.size());

        if (size != value.size())
            throw libnetwrk_exception("serialize: std::set size truncated.");

        uint32_t count = 0;
        for (auto& element : value) {
            if (count == size) break;

            serialize(buffer, element);
        }
    }

    template<typename Buffer, typename Type, typename... Args>
    void serialize(Buffer& buffer, const std::unordered_set<Type, Args...>& value) {
        uint32_t size = serialize(buffer, value.size());

        if (size != value.size())
            throw libnetwrk_exception("serialize: std::unordered_set size truncated.");

        uint32_t count = 0;
        for (auto& element : value) {
            if (count == size) break;

            serialize(buffer, element);
        }
    }

    template<typename Buffer, typename Key, typename Value>
    void serialize(Buffer& buffer, const std::map<Key, Value>& value) {
        uint32_t size = serialize(buffer, value.size());

        if (size != value.size())
            throw libnetwrk_exception("serialize: std::unordered_set size truncated.");

        uint32_t count = 0;
        for (auto& [kvp_key, kvp_value] : value) {
            if (count == size) break;

            serialize(buffer, kvp_key);
            serialize(buffer, kvp_value);
        }
    }

    template<typename Buffer, typename Key, typename Value>
    void serialize(Buffer& buffer, const std::unordered_map<Key, Value>& value) {
        uint32_t size = serialize(buffer, value.size());

        if (size != value.size())
            throw libnetwrk_exception("serialize: std::unordered_set size truncated.");

        uint32_t count = 0;
        for (auto& [kvp_key, kvp_value] : value) {
            if (count == size) break;

            serialize(buffer, kvp_key);
            serialize(buffer, kvp_value);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // DESERIALIZE
    
    template<typename Buffer, typename Type>
    requires is_primitive<Type>
    void deserialize(Buffer& buffer, Type& value) {
        internal::read(buffer, static_cast<uint8_t*>(static_cast<void*>(&value)), sizeof(Type));
    }

    template<typename Buffer, typename Type>
    requires is_serializable<Buffer, Type>
    void deserialize(Buffer& buffer, Type& value) {
        value.deserialize(buffer);
    }

    template<typename Buffer>
    void deserialize(Buffer& buffer, std::size_t& value) {
        if constexpr (sizeof(std::size_t) == 8) {
            uint32_t casted = 0U;
            deserialize(buffer, casted);
            value = static_cast<std::size_t>(casted);
        }
        else {
            internal::read(buffer, static_cast<uint8_t*>(static_cast<void*>(&value)), sizeof(std::size_t));
        }
    }

    template<typename Buffer>
    void deserialize(Buffer& buffer, std::string& value) {
        uint32_t size = 0;
        deserialize(buffer, size);
        
        value.resize(size);
        internal::read(buffer, static_cast<uint8_t*>(static_cast<void*>(&value)), size);
    }

    template<typename Buffer, typename Type, size_t Size>
    void deserialize(Buffer& buffer, std::array<Type, Size>& value) {
        uint32_t size = 0;
        deserialize(buffer, size);

        if (size != value.size())
            throw libnetwrk_exception("deserialize: std::array size not the same.");

        if constexpr (is_primitive<Type>) {
            internal::read(buffer, static_cast<uint8_t*>(static_cast<void*>(value.data())), size * sizeof(Type));
        }   
        else {
            for (uint32_t i = 0; i < size; i++) {
                deserialize(buffer, value[i]);
            }
        }
    }

    template<typename Buffer, typename Type>
    requires marked_supported_vector<Type>
    void deserialize(Buffer& buffer, std::vector<Type>& value) {
        uint32_t size = 0;
        deserialize(buffer, size);

        value.resize(size);

        if constexpr (is_primitive<Type>) {   
            internal::read(buffer, static_cast<uint8_t*>(static_cast<void*>(value.data())), size * sizeof(Type));
        }
        else {
            for (uint32_t i = 0; i < size; i++) {
                deserialize(buffer, value[i]);
            }
        }
    }

    template<typename Buffer, typename Type>
    void deserialize(Buffer& buffer, std::deque<Type>& value) {
        uint32_t size = 0;
        deserialize(buffer, size);

        value.clear();

        for (uint32_t i = 0; i < size; i++) {
            value.push_back({});
            deserialize(buffer, value.back());
        }
    }

    template<typename Buffer, typename Type>
    void deserialize(Buffer& buffer, std::list<Type>& value) {
        uint32_t size = 0;
        deserialize(buffer, size);

        value.clear();

        for (uint32_t i = 0; i < size; i++) {
            value.push_back({});
            deserialize(buffer, value.back());
        }
    }

    template<typename Buffer, typename Type, typename... Args>
    void deserialize(Buffer& buffer, std::set<Type, Args...>& value) {
        uint32_t size = 0;
        deserialize(buffer, size);

        value.clear();
        
        for (uint32_t i = 0; i < size; i++) {
            Type element{};
            deserialize(buffer, element);
            value.insert(element);
        }
    }

    template<typename Buffer, typename Type, typename... Args>
    void deserialize(Buffer& buffer, std::unordered_set<Type, Args...>& value) {
        uint32_t size = 0;
        deserialize(buffer, size);

        value.clear();

        for (uint32_t i = 0; i < size; i++) {
            Type element{};
            deserialize(buffer, element);
            value.insert(element);
        }
    }

    template<typename Buffer, typename Key, typename Value>
    void deserialize(Buffer& buffer, std::map<Key, Value>& value) {
        uint32_t size = 0;
        deserialize(buffer, size);

        value.clear();

        for (uint32_t i = 0; i < size; i++) {
            Key key{};
            deserialize(buffer, key);
            
            value[key] = {};
            deserialize(buffer, value[key]);
        }
    }

    template<typename Buffer, typename Key, typename Value>
    void deserialize(Buffer& buffer, std::unordered_map<Key, Value>& value) {
        uint32_t size = 0;
        deserialize(buffer, size);

        value.clear();

        for (uint32_t i = 0; i < size; i++) {
            Key key{};
            deserialize(buffer, key);

            value[key] = {};
            deserialize(buffer, value[key]);
        }
    }
}