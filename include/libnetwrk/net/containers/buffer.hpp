#pragma once

#include "libnetwrk/exceptions/libnetwrk_exception.hpp"

#include <cstdint>

namespace libnetwrk {
    class buffer {
    public:
        using value_t = uint8_t;

    public:
        buffer()              = default;
        buffer(const buffer&) = default;
        buffer(buffer&&)      = default;

        buffer& operator=(const buffer&) = default;
        buffer& operator=(buffer&&)      = default;

    public:
        virtual value_t* data()  = 0;
        virtual uint32_t size()  = 0;
        virtual void     clear() = 0;

        friend uint32_t& get_buffer_read_index(buffer& buffer) {
            return buffer.m_read_index;
        }

    protected:
        uint32_t m_read_index = 0U;
    };
}
