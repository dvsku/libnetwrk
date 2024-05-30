#pragma once

#include "libnetwrk/net/enum/enum_bitmask_operators.hpp"

#include <cstdint>

namespace libnetwrk {
    enum class service_status : uint8_t {
        stopped  = 0,
        starting = 1,
        started  = 2,
        stopping = 3
    };

    enum class send_flags : uint8_t {
        none         = 0,
        keep_message = 1 << 0       // Make message reusable (copy instead of move upon sending)
    };

    enum class disconnect_code : uint8_t {
        unspecified           = 0,
        authentication_failed = 1
    };
}
