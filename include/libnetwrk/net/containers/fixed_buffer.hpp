#pragma once

#include "libnetwrk/net/containers/buffer.hpp"

#include <array>

namespace libnetwrk {
    template<size_t Size>
    class fixed_buffer : public buffer {
    public:
        using value_t        = uint8_t;
        using container_t    = std::array<value_t, Size>;
        using iterator       = container_t::iterator;
        using const_iterator = container_t::const_iterator;
        using fixed_buffer_t = fixed_buffer<Size>;

    public:
        fixed_buffer()                      = default;
        fixed_buffer(const fixed_buffer_t&) = default;
        fixed_buffer(fixed_buffer_t&&)      = default;

        fixed_buffer_t& operator=(const fixed_buffer_t&) = default;
        fixed_buffer_t& operator=(fixed_buffer_t&&)      = default;

    public:
        void clear() override {
            m_read_index = 0;
            m_write_index = 0;
        }

        friend container_t& get_buffer_underlying(fixed_buffer_t& buffer) {
            return buffer.m_container;
        }

        friend size_t& get_buffer_write_index(fixed_buffer_t& buffer) {
            return buffer.m_write_index;
        }

    private:
        size_t      m_write_index = 0U;
        container_t m_container   = {};
    };
}
