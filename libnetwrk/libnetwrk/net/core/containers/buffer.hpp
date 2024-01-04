#pragma once

#include "libnetwrk/net/core/exceptions/libnetwrk_exception.hpp"

#include <vector>

namespace libnetwrk {
    template<typename Serialize>
    class buffer {
    public:
        using value_t        = uint8_t;
        using container_t    = std::vector<value_t>;
        using iterator       = std::vector<uint8_t>::iterator;
        using const_iterator = std::vector<uint8_t>::const_iterator;
        using serializer_t   = Serialize;
        using buffer_t       = buffer<serializer_t>;

    public:
        buffer()                = default;
        buffer(const buffer_t&) = default;

        buffer(buffer_t&& rhs) noexcept {
            this->m_data   = std::move(rhs.m_data);
            this->m_offset = rhs.m_offset;

            rhs.m_offset = 0U;
        }

        buffer(const_iterator first, const_iterator last) {
            m_data = container_t(first, last);
        }

        buffer_t& operator=(const buffer_t&) = default;

        buffer_t& operator=(buffer_t&& rhs) noexcept {
            if (this == &rhs) return *this;
                
            this->m_data   = std::move(rhs.m_data);
            this->m_offset = rhs.m_offset;

            rhs.m_offset = 0U;

            return *this;
        }

    public:
        container_t& underlying() {
            return m_data;
        }

        ///////////////////////////////////////////////////////////////////
        // ELEMENT ACCESS

        const value_t& operator [] (const size_t pos) const {
            return m_data[pos];
        }

        value_t* data() {
            return m_data.data();
        }

        const value_t* data() const {
            return m_data.data();
        }

        /// <summary>
        /// Get range from current read offset
        /// </summary>
        /// <param name="size"></param>
        /// <exception cref="libnetwrk_exception">thrown when accessing data out of bounds</exception>
        /// <returns>buffer with requested data</returns>
        buffer_t get_range(size_t size) {
            buffer_t buffer;
            buffer.resize(size);

            get_range(buffer.data(), size);
            return buffer;
        }

        /// <summary>
        /// Get range from current read offset into dst. dst must be large enough to accept the range or
        /// the behaviour is undefined. 
        /// </summary>
        /// <param name="dst">destination addr</param>
        /// <param name="size">size of range</param>
        /// <exception cref="libnetwrk_exception">thrown when accessing data out of bounds</exception>
        void get_range(void* dst, size_t size) {
            if (!dst)
                throw libnetwrk_exception("dst is null");

            if (m_offset + size > m_data.size())
                throw libnetwrk_exception("out of bounds");

            std::memcpy(dst, m_data.data() + m_offset, size);
            _advance(size);
        }

        ///////////////////////////////////////////////////////////////////
        // CAPACITY

        bool empty() const {
            return m_data.empty();
        }

        size_t size() const {
            return m_data.size();
        }

        void resize(size_t new_size) {
            m_data.resize(new_size);
        }

        ///////////////////////////////////////////////////////////////////
        // MODIFIERS

        void clear() {
            m_data.clear();
            reset_read_offset();
        }

        void reset_read_offset() {
            m_offset = 0;
        }

        void push_back(const buffer_t& buffer) {
            push_back(buffer.data(), buffer.size());
        }

        void push_back(const void* src, const size_t size) {
            m_data.insert(end(), (uint8_t*)src, (uint8_t*)src + size);
        }

        void swap_at(const buffer_t& buffer, const size_t offset) {
            swap_at(buffer.data(), buffer.size(), offset);
        }

        void swap_at(const void* src, const size_t size, const size_t offset) {
            if (!src)
                throw libnetwrk_exception("src is null");

            if (offset + size > m_data.size())
                throw libnetwrk_exception("out of bounds");

            std::memcpy(m_data.data() + offset, src, size);
        }

        ///////////////////////////////////////////////////////////////////
        // Iterators
        ///////////////////////////////////////////////////////////////////

        iterator begin() {
            return m_data.begin();
        }

        const_iterator begin() const {
            return m_data.begin();
        }

        iterator end() {
            return m_data.end();
        }

        const_iterator end() const {
            return m_data.end();
        }

        ///////////////////////////////////////////////////////////////////
        // SERIALIZATION

        template <typename T>
        friend buffer_t& operator<<(buffer_t& buffer, const T& value) {
            serializer_t::serialize(buffer, value);
            return buffer;
        }

        ///////////////////////////////////////////////////////////////////
        // DESERIALIZATION

        template <typename T>
        friend buffer_t& operator>>(buffer_t& buffer, T& value) {
            serializer_t::deserialize(buffer, value);
            return buffer;
        }

    protected:
        container_t m_data;
        size_t      m_offset = 0;

    private:
        void _advance(size_t value) {
            m_offset += value;
        }
    };
}
