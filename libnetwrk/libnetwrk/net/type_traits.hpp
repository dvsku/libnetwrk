#pragma once

#include "libnetwrk/net/common/serialization/serializable.hpp"

namespace libnetwrk {
    template<typename T, typename Tserialize>
    concept is_serializable = std::is_base_of<libnetwrk::serializable<Tserialize>, T>::value;

    template<typename T>
    concept is_enum = std::is_enum<T>::value;

    template<typename T>
    concept is_arithmentic_or_enum = std::is_arithmetic<T>::value || std::is_enum<T>::value;
}
