#pragma once

#include "libnetwrk/exceptions/libnetwrk_exception.hpp"

#include <cstdint>

namespace libnetwrk {
    class buffer {
    public:
        buffer()              = default;
        buffer(const buffer&) = default;
        buffer(buffer&&)      = default;

        buffer& operator=(const buffer&) = default;
        buffer& operator=(buffer&&)      = default;

    public:
        virtual void clear() = 0;

        friend size_t& get_buffer_read_index(buffer& buffer) {
            return buffer.m_read_index;
        }

    protected:
        size_t m_read_index = 0U;
    };
}
