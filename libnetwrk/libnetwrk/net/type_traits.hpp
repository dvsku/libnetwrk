#pragma once

#include <type_traits>

namespace libnetwrk {
    // Forward decl buffer

    template<typename Tserialize>
    class buffer;

    // Custom impl of std::convertable_to concept
    // since not all compilers support it yet

    template<typename From, typename To>
    concept is_convertable_to = std::is_convertible_v<From, To> 
        && requires { static_cast<To>(std::declval<From>()); };

    // Check for existance of 
    // template<typename T>
    // void serialize(buffer<T>& buffer> const;

    template <typename T, typename Tserialize>
    concept has_serialize_func = requires(const T obj, buffer<Tserialize>&buf) {
        { obj.serialize(buf) } -> is_convertable_to<void>;
    };

    // Check for existance of 
    // template<typename T>
    // void deserialize(buffer<T>& buffer>;

    template <typename T, typename Tserialize>
    concept has_deserialize_func = requires(T obj, buffer<Tserialize>&buf) {
        { obj.deserialize(buf) } -> is_convertable_to<void>;
        requires (!std::is_const_v<T>);
    };

    // Check if both serialize and deserialize exist

    template <typename T, typename Tserialize>
    concept is_serializable = has_serialize_func<T, Tserialize> && has_deserialize_func<T, Tserialize>;

    template<typename T>
    concept is_enum = std::is_enum<T>::value;

    template<typename T>
    concept is_arithmentic_or_enum = std::is_arithmetic<T>::value || std::is_enum<T>::value;
}
