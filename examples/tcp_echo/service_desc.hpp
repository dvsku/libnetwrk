#pragma once

#include "example_logging.hpp"

enum class commands : unsigned int {
    c2s_echo,
    s2c_echo
};

struct service_desc {
    using command_t = commands;
    using storage_t = libnetwrk::nothing;
};
