#pragma once

#include <type_traits>

namespace libnetwrk {
    template<typename T>
    concept is_enum = std::is_enum<T>::value;

    /*
        libnetwrk service descriptor.
    */
    template<typename T>
    concept is_libnetwrk_service_desc = requires {
        typename T::command_t;
        typename T::storage_t;

        std::is_enum<typename T::command_t>::value;
    };
}
