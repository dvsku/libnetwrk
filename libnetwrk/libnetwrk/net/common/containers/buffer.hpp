#ifndef LIBNETWRK_NET_COMMON_BUFFER_HPP
#define LIBNETWRK_NET_COMMON_BUFFER_HPP

#include <vector>

#include "libnetwrk/net/common/serialization/type_traits.hpp"

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
			uint32_t m_offset = 0;

		public:
			buffer() {}

			buffer(const_iterator first, const_iterator last) {
				m_data = std::vector<uint8_t>(first, last);
			}

			///////////////////////////////////////////////////////////////////
			// Element access
			///////////////////////////////////////////////////////////////////

			uint8_t& operator [] (const size_t pos) {
				return m_data[pos];
			}

			const uint8_t& operator [] (const size_t pos) const {
				return m_data[pos];
			}

			uint8_t& front() {
				return m_data.front();
			}

			const uint8_t& front() const {
				return m_data.front();
			}

			uint8_t& back() {
				return m_data.back();
			}

			const uint8_t& back() const {
				return m_data.back();
			}

			uint8_t* data() {
				return m_data.data();
			}

			const uint8_t* data() const {
				return m_data.data();
			}

			buffer_t get_range(size_t start, size_t count) {
				return buffer_t(m_data.begin() + start, m_data.begin() + start + count);
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

			size_t max_size() const {
				return m_data.max_size();
			}

			void reserve(size_t new_capacity) {
				m_data.reserve(new_capacity);
			}

			size_t capacity() const {
				return m_data.capacity();
			}

			void shrink_to_fit() {
				m_data.shrink_to_fit();
			}

			///////////////////////////////////////////////////////////////////
			// Modifiers
			///////////////////////////////////////////////////////////////////

			void clear() {
				m_data.clear();
			}

			void push_back(const uint8_t& value) {
				m_data.push_back(value);
			}

			void push_back(const buffer_t& buffer) {
				m_data.insert(end(), buffer.begin(), buffer.end());
			}

			void push_back(const void* src, const size_t size) {
				uint8_t* ptr = (uint8_t*)src;
				
				for (size_t i = 0; i < size; i++) {
					push_back(*ptr);
					ptr++;
				}
			}

			void pop_back() {
				m_data.pop_back();
			}

			void resize(const size_t new_size) {
				m_data.resize(new_size);
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
			// Offset
			///////////////////////////////////////////////////////////////////

			const uint32_t offset() {
				return m_offset;
			}

			void advance(uint32_t value) {
				m_offset += value;
			}

			///////////////////////////////////////////////////////////////////
			// Serialization
			///////////////////////////////////////////////////////////////////

			template <typename T>
			friend buffer_t& operator << (buffer_t& buffer, const T& value) {
				serializer::serialize(buffer, value);
				return buffer;
			}

			///////////////////////////////////////////////////////////////////
			// Deserialization
			///////////////////////////////////////////////////////////////////

			template <typename T>
			friend buffer_t& operator >> (buffer_t& buffer, T& obj) {
				serializer::deserialize(buffer, obj);
			    return buffer;
			}
	};
}

#endif