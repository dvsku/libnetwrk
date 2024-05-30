#pragma once

#include "libnetwrk/net/serialize/serialize_internal.hpp"

namespace libnetwrk::serialize {
    template<typename Buffer, typename Type>
    static void serialize(Buffer& buffer, const Type& value) {
        libnetwrk::serialize::internal::serialize(buffer, value);
    }

    template<typename Buffer, typename Type>
    static void deserialize(Buffer& buffer, Type& obj) {
        libnetwrk::serialize::internal::deserialize(buffer, obj);
    }
}

namespace libnetwrk {
    template<typename Buffer, typename Value>
    requires libnetwrk::serialize::internal::is_supported_buffer<Buffer>
    Buffer& operator<<(Buffer& buffer, const Value& value) {
        libnetwrk::serialize::serialize(buffer, value);
        return buffer;
    }

    template<typename Buffer, typename Value>
    requires libnetwrk::serialize::internal::is_supported_buffer<Buffer>
    Buffer& operator>>(Buffer& buffer, Value& value) {
        libnetwrk::serialize::deserialize(buffer, value);
        return buffer;
    }
}