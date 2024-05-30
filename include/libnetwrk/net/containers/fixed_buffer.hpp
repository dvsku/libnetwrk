#pragma once

#include "libnetwrk/net/containers/buffer.hpp"

#include <array>

namespace libnetwrk::serialize {
    template<typename Buffer, typename Type>
    void serialize(Buffer& buffer, const Type& value);

    template<typename Buffer, typename Type>
    void deserialize(Buffer& buffer, Type& obj);
}

namespace libnetwrk {
    struct fixed_size_buffer {};

    template<uint32_t Size>
    class fixed_buffer : public buffer, public fixed_size_buffer {
    public:
        using value_t        = buffer::value_t;
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
        value_t* data() override {
            return m_container.data();
        }

        uint32_t size() override {
            return (uint32_t)m_container.size();
        }

        void clear() override {
            m_read_index = 0;
            m_write_index = 0;
        }

        bool empty() {
            return m_write_index == 0;
        }

        iterator begin() {
            return m_container.begin();
        }

        const_iterator begin() const {
            return m_container.begin();
        }

        iterator end() {
            return m_container.end();
        }

        const_iterator end() const {
            return m_container.end();
        }

        container_t& underlying() {
            return m_container;
        }

        friend uint32_t& get_buffer_write_index(fixed_buffer_t& buffer) {
            return buffer.m_write_index;
        }

        template<typename Value>
        fixed_buffer_t& operator<<(const Value& value) {
            libnetwrk::serialize::serialize(*this, value);
            return *this;
        }

        template<typename Value>
        fixed_buffer_t& operator>>(Value& value) {
            libnetwrk::serialize::deserialize(*this, value);
            return *this;
        }

    private:
        uint32_t    m_write_index = 0U;
        container_t m_container   = {};
    };
}
