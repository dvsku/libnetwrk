#pragma once

#include <stdexcept>
#include <string>

namespace libnetwrk {
    class libnetwrk_exception : public std::runtime_error {
    public:
        libnetwrk_exception()
            : std::runtime_error("Undefined.") {}

        libnetwrk_exception(const std::string& message)
            : std::runtime_error(message) {}
    };
}
