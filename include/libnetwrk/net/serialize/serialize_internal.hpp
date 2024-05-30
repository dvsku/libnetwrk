#pragma once

#include "libnetwrk/net/containers/buffer.hpp"
#include "libnetwrk/net/serialize/serialize_adapters.hpp"
#include "libnetwrk/net/serialize/serialize_endian.hpp"
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

    template<typename From, typename To>
    concept convertible_to = requires {
        requires std::is_convertible_v<From, To> &&
                 requires { static_cast<To>(std::declval<From>()); };
    };

    template<typename Buffer>
    concept supported_buffer = std::derived_from<Buffer, libnetwrk::buffer>;

    ////////////////////////////////////////////////////////////////////////////
    // SUPPORTED

    template<typename Type>
    concept primitive = requires {
        requires std::same_as<Type, bool>     ||
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
                 std::same_as<Type, double>   ||
                 libnetwrk::is_enum<Type>;
    };

    template<typename Buffer, typename Type>
    concept user_defined_serialize = requires {
        requires supported_buffer<Buffer>
        && requires(const Type value, Buffer& buffer) {
            { value.serialize(buffer) } -> convertible_to<void>;
        }
        && requires(Type value, Buffer& buffer) {
            { value.deserialize(buffer) } -> convertible_to<void>;
            requires (!std::is_const_v<Type>);
        };
    };

    template<typename Type>
    concept is_std_array = requires {
        requires (
            requires { typename std::tuple_size<Type>::type; }       &&
            requires { typename std::tuple_element<0, Type>::type; } &&
            std::same_as<Type, std::array<typename Type::value_type, std::tuple_size<Type>::value>>
        );
    };

    template<typename Type>
    concept contiguous_containers = requires {
        requires ( 
            std::same_as<Type, std::vector<typename Type::value_type>> && 
            !std::same_as<typename Type::value_type, bool> 
        ) 
        || is_std_array<Type>;
    };

    template<typename Type>
    concept containers = requires {
        requires contiguous_containers<Type>                               ||
                 std::same_as<Type, std::deque<typename Type::value_type>> ||
                 std::same_as<Type, std::list<typename Type::value_type>>  ||
                 std::same_as<Type, std::set<typename Type::value_type>>   ||
                 std::same_as<Type, std::unordered_set<typename Type::value_type>>;
    };

    template<typename Type>
    concept kvp_containers = requires {
        requires std::same_as<Type, std::map<typename Type::key_type, typename Type::mapped_type>> ||
                 std::same_as<Type, std::unordered_map<typename Type::key_type, typename Type::mapped_type>>;
    };

    ////////////////////////////////////////////////////////////////////////////
    // SERIALIZABLE

    

}

namespace libnetwrk::serialize::internal {
    template<typename Buffer, typename Type>
    concept serialize_supported = requires {
        requires primitive<Type>                      ||
                 user_defined_serialize<Buffer, Type> ||
                 containers<Type>                     ||
                 kvp_containers<Type>                 ||
                 std::same_as<Type, std::string>      || 
                 std::same_as<Type, char*>;
    };

    template<typename Buffer, typename Type>
    concept serialize_unsupported = !serialize_supported<Buffer, Type>;

    ////////////////////////////////////////////////////////////////////////////
    // UNSUPPORTED

    template<typename Buffer, typename Type>
    requires serialize_unsupported<Buffer, Type>
    void serialize(Buffer& buffer, const Type& value) {
        static_assert(assert_force_false<Type>, "Called serialize() or << on a buffer with an unsupported type as the arg.");
    }

    template<typename Buffer, typename Type>
    requires serialize_unsupported<Buffer, Type>
    void deserialize(Buffer& buffer, Type& value) {
        static_assert(assert_force_false<Type>, "Called deserialize() or >> on a buffer with an unsupported type as the arg.");
    }

    ////////////////////////////////////////////////////////////////////////////
    // SERIALIZE

    template<typename Buffer, typename Type>
    requires primitive<Type>
    void serialize(Buffer& buffer, const Type& value) {
        if constexpr (enforce_endianness<Type>) {
            Type le = value;
            internal::byte_swap(le);
            internal::write(buffer, static_cast<const uint8_t*>(static_cast<const void*>(&le)), sizeof(Type));
        }
        else {
            internal::write(buffer, static_cast<const uint8_t*>(static_cast<const void*>(&value)), sizeof(Type));
        }
    }

    template<typename Buffer, typename Type>
    requires user_defined_serialize<Buffer, Type>
    void serialize(Buffer& buffer, const Type& value) {
        value.serialize(buffer);
    }

    template<typename Buffer>
    void serialize(Buffer& buffer, const std::string& value) {
        uint32_t size = static_cast<uint32_t>(value.size());

        if (size != value.size())
            throw libnetwrk_exception("serialize: size truncated.");

        serialize(buffer, size);
        internal::write(buffer, static_cast<const uint8_t*>(static_cast<const void*>(value.data())), size);
    }

    template<typename Buffer>
    void serialize(Buffer& buffer, const char* value) {
        uint32_t size = static_cast<uint32_t>(strlen(value));

        serialize(buffer, size);
        internal::write(buffer, static_cast<const uint8_t*>(static_cast<const void*>(value)), size);
    }

    template<typename Buffer, typename Type>
    requires containers<Type>
    void serialize(Buffer& buffer, const Type& value) {
        uint32_t size = static_cast<uint32_t>(value.size());

        if (size != value.size())
            throw libnetwrk_exception("serialize: size truncated.");

        serialize(buffer, size);

        constexpr auto use_memcpy = (
            contiguous_containers<Type>          && 
            primitive<typename Type::value_type> && 
            !enforce_endianness<typename Type::value_type>
        );

        if constexpr (use_memcpy) {
            internal::write(buffer,
                            static_cast<const uint8_t*>(static_cast<const void*>(value.data())),
                            size * sizeof(typename Type::value_type));
        }
        else {
            for (auto& element : value) {
                serialize(buffer, element);
            }
        }
    }

    template<typename Buffer, typename Type>
    requires kvp_containers<Type>
    void serialize(Buffer& buffer, const Type& value) {
        uint32_t size = static_cast<uint32_t>(value.size());

        if (size != value.size())
            throw libnetwrk_exception("serialize: size truncated.");

        serialize(buffer, size);

        for (auto& [kvp_key, kvp_value] : value) {
            serialize(buffer, kvp_key);
            serialize(buffer, kvp_value);
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // DESERIALIZE
    
    template<typename Buffer, typename Type>
    requires primitive<Type>
    void deserialize(Buffer& buffer, Type& value) {
        internal::read(buffer, static_cast<uint8_t*>(static_cast<void*>(&value)), sizeof(Type));

        if constexpr (enforce_endianness<Type>) {
            internal::byte_swap(value);
        }
    }

    template<typename Buffer, typename Type>
    requires user_defined_serialize<Buffer, Type>
    void deserialize(Buffer& buffer, Type& value) {
        value.deserialize(buffer);
    }

    template<typename Buffer>
    void deserialize(Buffer& buffer, std::string& value) {
        uint32_t size = 0;
        deserialize(buffer, size);
        
        value.resize(size);
        internal::read(buffer, static_cast<uint8_t*>(static_cast<void*>(value.data())), size);
    }

    template<typename Buffer, typename Type>
    requires containers<Type>
    void deserialize(Buffer& buffer, Type& value) {
        uint32_t size = 0;
        deserialize(buffer, size);

        if constexpr (is_std_array<Type>) {
            if (size != value.size())
                throw libnetwrk_exception("deserialize: std::array size not the same.");
        }
        else if constexpr (std::same_as<Type, std::vector<typename Type::value_type>>) {
            value.resize(size);
        }
        else {
            value.clear();
        }

        constexpr auto use_memcpy = (
            contiguous_containers<Type>          && 
            primitive<typename Type::value_type> && 
            !enforce_endianness<typename Type::value_type>
        );

        if constexpr (use_memcpy) {
            internal::read(buffer, static_cast<uint8_t*>(static_cast<void*>(value.data())), size * sizeof(typename Type::value_type));
        }
        else {
            for (uint32_t i = 0; i < size; i++) {
                if constexpr (contiguous_containers<Type>) 
                {
                    deserialize(buffer, value[i]);
                }
                else if constexpr (std::same_as<Type, std::deque<typename Type::value_type>> ||
                    std::same_as<Type, std::list<typename Type::value_type>>) 
                {
                    value.push_back({});
                    deserialize(buffer, value.back());
                }
                else {
                    typename Type::value_type element{};
                    deserialize(buffer, element);
                    value.insert(element);
                }
            }
        }
    }

    template<typename Buffer, typename Type>
    requires kvp_containers<Type>
    void deserialize(Buffer& buffer, Type& value) {
        uint32_t size = 0;
        deserialize(buffer, size);

        value.clear();

        for (uint32_t i = 0; i < size; i++) {
            typename Type::key_type key{};
            deserialize(buffer, key);

            value[key] = {};
            deserialize(buffer, value[key]);
        }
    }
}