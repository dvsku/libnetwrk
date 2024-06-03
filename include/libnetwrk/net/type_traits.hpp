#pragma once

#include <type_traits>

namespace libnetwrk {
    template<typename T>
    concept is_enum = std::is_enum<T>::value;

    template<typename Desc>
    concept libnetwrk_desc = requires {
        typename Desc::command_t;

        requires is_enum<typename Desc::command_t>;
    };
}
