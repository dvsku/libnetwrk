#pragma once

#include <cstdint>

namespace libnetwrk {
    struct nothing {};

    struct default_service_desc {
        using command_t = uint32_t;
        using storage_t = nothing;
    };
}
