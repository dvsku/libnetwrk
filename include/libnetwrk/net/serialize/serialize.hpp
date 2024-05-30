#pragma once

#include "libnetwrk/net/serialize/serialize_internal.hpp"

namespace libnetwrk::serialize {
    template<typename Buffer, typename Type>
    void serialize(Buffer& buffer, const Type& value) {
        libnetwrk::serialize::internal::serialize(buffer, value);
    }

    template<typename Buffer, typename Type>
    void deserialize(Buffer& buffer, Type& obj) {
        libnetwrk::serialize::internal::deserialize(buffer, obj);
    }
}
