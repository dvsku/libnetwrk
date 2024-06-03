#pragma once

#include <type_traits>

namespace {
    template <typename, typename = std::void_t<>>
    struct has_storage_type_typename : std::false_type {};

    template <typename T>
    struct has_storage_type_typename<T, std::void_t<typename T::storage_t>> : std::true_type {};
}

namespace libnetwrk {
    template<typename T>
    concept is_enum = std::is_enum<T>::value;

    template<typename Desc>
    concept libnetwrk_desc = requires {
        typename Desc::command_t;

        requires is_enum<typename Desc::command_t>;
    };

    template <typename Desc>
    constexpr bool desc_has_storage_type = has_storage_type_typename<Desc>::value;
}
