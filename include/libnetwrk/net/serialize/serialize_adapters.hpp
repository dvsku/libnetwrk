#pragma once

#include "libnetwrk/net/containers/fixed_buffer.hpp"
#include "libnetwrk/net/containers/dynamic_buffer.hpp"
#include "libnetwrk/exceptions/libnetwrk_exception.hpp"

#include <cstring>

namespace libnetwrk::serialize::internal {
    ////////////////////////////////////////////////////////////////////////////
    // DYNAMIC BUFFER

    void read(dynamic_buffer& buffer, uint8_t* destination, uint32_t size) {
        auto& underlying = buffer.underlying();
        auto& read_index = get_buffer_read_index(buffer);

        if (read_index + size > underlying.size())
            throw libnetwrk_exception("dynamic_buffer: tried to read outside bounds.");

        std::memcpy(destination, underlying.data() + read_index, size);
        read_index += size;
    }

    void write(dynamic_buffer& buffer, const uint8_t* data, uint32_t size) {
        auto& underlying = buffer.underlying();

        underlying.insert(underlying.end(), data, data + size);
    }

    ////////////////////////////////////////////////////////////////////////////
    // FIXED BUFFER

    template<uint32_t Size>
    void read(fixed_buffer<Size>& buffer, uint8_t* destination, uint32_t size) {
        auto& underlying  = buffer.underlying();
        auto& write_index = get_buffer_write_index(buffer);
        auto& read_index  = get_buffer_read_index(buffer);

        if (read_index + size > write_index)
            throw libnetwrk_exception("fixed_buffer: tried to read outside bounds.");

        std::memcpy(destination, underlying.data() + read_index, size);
        read_index += size;
    }

    template<uint32_t Size>
    void write(fixed_buffer<Size>& buffer, const uint8_t* data, uint32_t size) {
        auto& underlying  = buffer.underlying();
        auto& write_index = get_buffer_write_index(buffer);

        if (write_index + size > underlying.size())
            throw libnetwrk_exception("fixed_buffer: tried to write outside bounds.");

        std::memcpy(underlying.data() + write_index, data, size);
        write_index += size;
    }
}