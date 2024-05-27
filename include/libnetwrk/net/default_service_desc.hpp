#pragma once

#include "libnetwrk/net/serialization/bin_serialize.hpp"

#include <cstdint>

namespace libnetwrk {
    struct nothing {};

    struct default_service_desc {
        using command_t   = uint32_t;
        using serialize_t = bin_serialize;
        using storage_t   = nothing;
    };
}
