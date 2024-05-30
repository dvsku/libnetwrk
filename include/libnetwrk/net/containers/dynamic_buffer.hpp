#pragma once

#include "libnetwrk/net/containers/buffer.hpp"

#include <vector>

namespace libnetwrk::serialize {
    template<typename Buffer, typename Type>
    void serialize(Buffer& buffer, const Type& value);

    template<typename Buffer, typename Type>
    void deserialize(Buffer& buffer, Type& obj);
}

namespace libnetwrk {
    class dynamic_buffer : public buffer {
    public:
        using value_t        = buffer::value_t;
        using container_t    = std::vector<value_t>;
        using iterator       = container_t::iterator;
        using const_iterator = container_t::const_iterator;

    public:
        dynamic_buffer()                      = default;
        dynamic_buffer(const dynamic_buffer&) = default;
        dynamic_buffer(dynamic_buffer&&)      = default;

        dynamic_buffer(uint32_t size) {
            m_container.resize((size_t)size);
        }

        dynamic_buffer& operator=(const dynamic_buffer&) = default;
        dynamic_buffer& operator=(dynamic_buffer&&)      = default;

    public:
        value_t* data() override {
            return m_container.data();
        }

        uint32_t size() override {
            return (uint32_t)m_container.size();
        }

        void clear() override {
            m_read_index = 0;
            m_container.clear();
        }

        bool empty() {
            return m_container.size();
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

        template<typename Value>
        dynamic_buffer& operator<<(const Value& value) {
            libnetwrk::serialize::serialize(*this, value);
            return *this;
        }

        template<typename Value>
        dynamic_buffer& operator>>(Value& value) {
            libnetwrk::serialize::deserialize(*this, value);
            return *this;
        }

    private:
        container_t m_container;
    };
}
