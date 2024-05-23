#pragma once

#include <cstdint>

namespace libnetwrk {
    #define LIBNETWRK_SET_FLAG(_flags, _flag) _flags =| _flag
    #define LIBNETWRK_FLAG_SET(_flags, _flag) _flags  & _flag

    enum service_status : uint8_t {
        stopped  = 0,
        starting = 1,
        started  = 2,
        stopping = 3
    };

    enum send_flags : uint8_t {
        none         = 0,
        keep_message = 1 << 0       // Make message reusable (copy instead of move upon sending)
    };
}
