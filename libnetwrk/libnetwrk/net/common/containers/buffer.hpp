#ifndef LIBNETWRK_NET_COMMON_BUFFER_HPP
#define LIBNETWRK_NET_COMMON_BUFFER_HPP

#include <vector>

#include "libnetwrk/net/type_traits.hpp"
#include "libnetwrk/net/common/exceptions/libnetwrk_exception.hpp"

namespace libnetwrk::net::common {
    // Forward declare
    struct binary_serializer;

    template<typename serializer = binary_serializer>
    class buffer {
        public:
            typedef std::vector<uint8_t>::iterator iterator;
            typedef std::vector<uint8_t>::const_iterator const_iterator;
            typedef buffer<serializer> buffer_t;

        protected:
            std::vector<uint8_t> m_data;
            size_t m_offset = 0;

        public:
            buffer() {
                m_data.resize(0);
            }

            buffer(const_iterator first, const_iterator last) {
                m_data = std::vector<uint8_t>(first, last);
            }

            ///////////////////////////////////////////////////////////////////
            // Element access
            ///////////////////////////////////////////////////////////////////

            std::vector<uint8_t>& container() {
                return m_data;
            }

            const uint8_t& operator [] (const size_t pos) const {
                return m_data[pos];
            }

            uint8_t* data() {
                return m_data.data();
            }

            const uint8_t* data() const {
                return m_data.data();
            }

            /// <summary>
            /// Get range from current read offset
            /// </summary>
            /// <param name="size"></param>
            /// <exception cref="libnetwrk_exception">thrown when accessing data out of bounds</exception>
            /// <returns>buffer with requested data</returns>
            buffer_t get_range(size_t size) {
                if (m_offset + size > m_data.size())
                    throw libnetwrk_exception("out of bounds, buffer doesn't have more data");

                buffer_t b(m_data.begin() + m_offset, m_data.begin() + m_offset + size);
                advance(size);
                return b;
            }

            /// <summary>
            /// Get range from current read offset into dst. dst must be large enough to accept the range or
            /// the behaviour is undefined. 
            /// </summary>
            /// <param name="dst">destination addr</param>
            /// <param name="size">size of range</param>
            /// <exception cref="libnetwrk_exception">thrown when accessing data out of bounds</exception>
            void get_range(void* dst, size_t size) {
                if (m_offset + size > m_data.size())
                    throw libnetwrk_exception("out of bounds, buffer doesn't have more data");

                std::memcpy(dst, m_data.data() + m_offset, size);
                advance(size);
            }

            ///////////////////////////////////////////////////////////////////
            // Capacity
            ///////////////////////////////////////////////////////////////////

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
            // Modifiers
            ///////////////////////////////////////////////////////////////////

            void clear() {
                m_data.clear();
                m_offset = 0;
            }

            void push_back(const buffer_t& buffer) {
                m_data.insert(end(), buffer.begin(), buffer.end());
            }

            void push_back(const void* src, const size_t size) {
                uint8_t* ptr = (uint8_t*)src;
                m_data.insert(end(), ptr, ptr + size);
            }

            void push_at(const buffer_t& buffer, const size_t offset) {
                m_data.insert(begin() + offset, buffer.begin(), buffer.end());
            }

            void push_at(const void* src, const size_t size, const size_t offset) {
                uint8_t* ptr = (uint8_t*)src;
                m_data.insert(begin() + offset, ptr, ptr + size);
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
            // Serialization
            ///////////////////////////////////////////////////////////////////

            /// <summary>
            /// Serialize obj
            /// </summary>
            template <typename T>
            friend buffer_t& operator << (buffer_t& buffer, const T& value) {
                serializer::serialize(buffer, value);
                return buffer;
            }

            ///////////////////////////////////////////////////////////////////
            // Deserialization
            ///////////////////////////////////////////////////////////////////

            /// <summary>
            /// Deserialize obj
            /// </summary>
            template <typename T>
            friend buffer_t& operator >> (buffer_t& buffer, T& obj) {
                serializer::deserialize(buffer, obj);
                return buffer;
            }

        private:
            void advance(size_t value) {
                m_offset += value;
            }
    };
}

#endif