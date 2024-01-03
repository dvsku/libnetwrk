#pragma once

#ifdef LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT

// Defines unsupported serialize and deserialize functions for containers that have a type.
// Functions will throw an exception during runtime if called.
// Useful for testing, not recommended for use outside of tests.
#define SERIALIZER_UNSUPPORTED_CONTAINER(container)                                                                                                 \
    template<typename T>                                                                                                                            \
    static void serialize(buffer_t& buffer, const container<T>& value) {                                                                            \
        throw libnetwrk::libnetwrk_exception("serialize() doesn't support " #container " containers");                                              \
    }                                                                                                                                               \
                                                                                                                                                    \
    template<typename T>                                                                                                                            \
    static void deserialize(buffer_t& buffer, container<T>& value) {                                                                                \
        throw libnetwrk::libnetwrk_exception("deserialize() doesn't support " #container " containers");                                            \
    }

// Defines unsupported serialize and deserialize functions for containers that have a key-value pair.
// Functions will throw an exception during runtime if called.
// Useful for testing, not recommended for use outside of tests.
#define SERIALIZER_UNSUPPORTED_KVP_CONTAINER(container)                                                                                             \
    template<typename Tkey, typename Tvalue>                                                                                                        \
    static void serialize(buffer_t& buffer, const container<Tkey, Tvalue>& value) {                                                                 \
        throw libnetwrk::libnetwrk_exception("serialize() doesn't support " #container " containers");                                              \
    }                                                                                                                                               \
                                                                                                                                                    \
    template<typename Tkey, typename Tvalue>                                                                                                        \
    static void deserialize(buffer_t& buffer, container<Tkey, Tvalue>& value) {                                                                     \
        throw libnetwrk::libnetwrk_exception("deserialize() doesn't support " #container " containers");                                            \
    }

#else

template <typename...>
inline constexpr bool always_false = false;

// Defines unsupported serialize and deserialize functions for containers that have a type.
// Functions will output an error during compilation if referenced.
#define SERIALIZER_UNSUPPORTED_CONTAINER(container)                                                                                                 \
    template<typename T>                                                                                                                            \
    static void serialize(buffer_t& buffer, const container<T>& value) {                                                                            \
        static_assert(always_false<container<T>>, "serialize() doesn't support " #container " containers");                                         \
    }                                                                                                                                               \
                                                                                                                                                    \
    template<typename T>                                                                                                                            \
    static void deserialize(buffer_t& buffer, container<T>& value) {                                                                                \
        static_assert(always_false<container<T>>, "deserialize() doesn't support " #container " containers");                                       \
    }

// Defines unsupported serialize and deserialize functions for containers that have a key-value pair.
// Functions will output an error during compilation if referenced.
#define SERIALIZER_UNSUPPORTED_KVP_CONTAINER(container)                                                                                             \
    template<typename Tkey, typename Tvalue>                                                                                                        \
    static void serialize(buffer_t& buffer, const container<Tkey, Tvalue>& value) {                                                                 \
        static_assert(always_false<container<Tkey, Tvalue>>, "serialize() doesn't support " #container " containers");                              \
    }                                                                                                                                               \
                                                                                                                                                    \
    template<typename Tkey, typename Tvalue>                                                                                                        \
    static void deserialize(buffer_t& buffer, container<Tkey, Tvalue>& value) {                                                                     \
        static_assert(always_false<container<Tkey, Tvalue>>, "deserialize() doesn't support " #container " containers");                            \
    }

#endif