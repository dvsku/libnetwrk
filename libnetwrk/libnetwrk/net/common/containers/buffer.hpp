#ifndef LIBNETWRK_NET_COMMON_BUFFER_HPP
#define LIBNETWRK_NET_COMMON_BUFFER_HPP

#include <vector>

namespace libnetwrk::net::common {
	class buffer {
		public:
			typedef std::vector<uint8_t>::iterator iterator;
			typedef std::vector<uint8_t>::const_iterator const_iterator;

		protected:
			std::vector<uint8_t> m_data;

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

			buffer get_range(size_t start, size_t count) {
				return buffer(m_data.begin() + start, m_data.begin() + start + count);
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

			void push_back(const buffer& buffer) {
				m_data.insert(end(), buffer.begin(), buffer.end());
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
	};
}

#endif