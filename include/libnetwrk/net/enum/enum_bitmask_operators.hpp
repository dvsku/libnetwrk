#pragma once

/*
    Code adapted from:
        https://github.com/p-ranav/alpaca
*/

#include <type_traits>

namespace libnetwrk {
    template<typename Enum>
    struct enum_bitmask_operators {
        static constexpr bool enabled = false;
    };

    template<typename Enum>
    concept enum_bitmask_operators_enabled = enum_bitmask_operators<Enum>::enabled;

    template<typename Enum>
    requires is_enum<Enum> && enum_bitmask_operators_enabled<Enum>
    constexpr Enum operator|(Enum lhs, Enum rhs) {
        using underlying = typename std::underlying_type<Enum>::type;

        return static_cast<Enum>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
    }

    template<typename Enum>
    requires is_enum<Enum>
    constexpr auto to_underlying(Enum e) noexcept {
        return static_cast<std::underlying_type_t<Enum>>(e);
    }

    template<typename Enum>
    requires is_enum<Enum>
    constexpr bool enum_has_flag(Enum flags, Enum flag) {
        using underlying = typename std::underlying_type<Enum>::type;

        return (static_cast<underlying>(flags) & static_cast<underlying>(flag)) == static_cast<underlying>(flag);
    }

    #define LIBNETWRK_ENABLE_ENUM_BITMASK_OPERATORS(e)  \
        template<>                                      \
        struct enum_bitmask_operators<e> {              \
            static constexpr bool enabled = true;       \
        }
}
