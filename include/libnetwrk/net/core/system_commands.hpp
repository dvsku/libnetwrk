#pragma once

#include <cstdint>

namespace libnetwrk {
    enum class system_command : uint16_t {
        c2s_verify,
        s2c_verify,
        s2c_verify_ok
    };
}
