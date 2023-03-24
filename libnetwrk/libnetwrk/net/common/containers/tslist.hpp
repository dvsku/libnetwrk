#ifndef LIBNETWRK_NET_COMMON_TSLIST_HPP
#define LIBNETWRK_NET_COMMON_TSLIST_HPP

#include <list>
#include <mutex>

#include "libnetwrk/net/type_traits.hpp"

#ifdef LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
	#include "libnetwrk/net/common/exceptions/libnetwrk_exception.hpp"
#endif

namespace libnetwrk::net::common {
	template <typename T> class tslist {
		protected:
			std::mutex m_mutex;
			std::list<T> m_list;

			tslist(const tslist<T>&) = delete;

		public:
			tslist() {
				m_list.resize(0);
			}

			virtual ~tslist() {
				clear();
			}

			///////////////////////////////////////////////////////////////////
			// Element access
			///////////////////////////////////////////////////////////////////

			const T& front() {
				const std::lock_guard<std::mutex> lock(m_mutex);
				return m_list.front();
			}

			const T& back() {
				const std::lock_guard<std::mutex> lock(m_mutex);
				return m_list.back();
			}

			///////////////////////////////////////////////////////////////////
			// Modifiers
			///////////////////////////////////////////////////////////////////

			T pop_front() {
				const std::lock_guard<std::mutex> lock(m_mutex);
				T t = std::move(m_list.front());
				m_list.pop_front();
				return t;
			}

			T pop_back() {
				const std::lock_guard<std::mutex> lock(m_mutex);
				T t = std::move(m_list.back());
				m_list.pop_back();
				return t;
			}

			void push_front(const T& element) {
				const std::lock_guard<std::mutex> lock(m_mutex);
				m_list.emplace_front(std::move(element));
			}

			void push_back(const T& element) {
				const std::lock_guard<std::mutex> lock(m_mutex);
				m_list.emplace_back(std::move(element));
			}

			void clear() {
				const std::lock_guard<std::mutex> lock(m_mutex);
				m_list.clear();
			}

			template <typename foreach_statement>
			void foreach(foreach_statement statement) {
				#ifdef LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
					if(!libnetwrk::net::is_signature<foreach_statement, void, T&>)
						throw libnetwrk::net::common::libnetwrk_exception(
							"tslist::foreach statement signature must be: void f(T&)");
				#else
					static_assert(libnetwrk::net::is_signature<foreach_statement, void, T&>,
						"tslist::foreach statement signature must be: void f(T&)");
				#endif	

				const std::lock_guard<std::mutex> lock(m_mutex);
				for (T& t : m_list) {
					statement(t);
				}
			}

			///////////////////////////////////////////////////////////////////
			// Capacity
			///////////////////////////////////////////////////////////////////

			bool empty() {
				const std::lock_guard<std::mutex> lock(m_mutex);
				return m_list.empty();
			}

			size_t count() {
				const std::lock_guard<std::mutex> lock(m_mutex);
				return m_list.size();
			}

	};
}

#endif