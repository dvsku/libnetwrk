#pragma once

#include "libnetwrk/net/containers/buffer.hpp"

#include <vector>

namespace libnetwrk {
    class dynamic_buffer : public buffer {
    public:
        using value_t        = uint8_t;
        using container_t    = std::vector<value_t>;
        using iterator       = container_t::iterator;
        using const_iterator = container_t::const_iterator;

    public:
        dynamic_buffer()                      = default;
        dynamic_buffer(const dynamic_buffer&) = default;
        dynamic_buffer(dynamic_buffer&&)      = default;

        dynamic_buffer(size_t size) {
            m_container.resize(size);
        }

        dynamic_buffer& operator=(const dynamic_buffer&) = default;
        dynamic_buffer& operator=(dynamic_buffer&&)      = default;

    public:
        void clear() override {
            m_read_index = 0;
            m_container.clear();
        }

        friend container_t& get_buffer_underlying(dynamic_buffer& buffer) {
            return buffer.m_container;
        }

    private:
        container_t m_container;
    };
}
