#pragma once

#include <type_traits>

namespace libnetwrk {
    template<typename Serialize>
    class buffer;

    /*
        Custom impl of std::convertible_to concept since not all compilers support it yet
    */
    template<typename From, typename To>
    concept is_convertible_to = std::is_convertible_v<From, To>
        && requires { static_cast<To>(std::declval<From>()); };

    /*
        Check for existence of
        template<typename T>
        void serialize(buffer<T>& buffer> const;
    */
    template<typename T, typename Serialize>
    concept has_serialize_func = requires(const T obj, buffer<Serialize>&buf) {
        { obj.serialize(buf) } -> is_convertible_to<void>;
    };

    /*
        Check for existence of
        template<typename T>
        void deserialize(buffer<T>& buffer>;
    */
    template<typename T, typename Serialize>
    concept has_deserialize_func = requires(T obj, buffer<Serialize>&buf) {
        { obj.deserialize(buf) } -> is_convertible_to<void>;
        requires (!std::is_const_v<T>);
    };

    /*
        Check if both serialize and deserialize exist
    */
    template<typename T, typename Serialize>
    concept is_serializable = has_serialize_func<T, Serialize> && has_deserialize_func<T, Serialize>;

    template<typename T>
    concept is_enum = std::is_enum<T>::value;

    template<typename T>
    concept is_arithmentic_or_enum = std::is_arithmetic<T>::value || std::is_enum<T>::value;

    /*
        libnetwrk service descriptor
    */
    template<typename T>
    concept is_libnetwrk_service_desc = requires {
        typename T::command_t;
        typename T::serialize_t;
        typename T::storage_t;

        std::is_enum<typename T::command_t>::value;
    };
}
