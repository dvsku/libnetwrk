#ifndef LIBNETWRK_NET_COMMON_TSDEQUE_HPP
#define LIBNETWRK_NET_COMMON_TSDEQUE_HPP

#include <deque>
#include <mutex>

namespace libnetwrk::net::common {
	template <typename T> class tsdeque {
		protected:
			std::mutex m_mutex_queue;
			std::mutex m_mutex_wait;
			std::condition_variable m_cv_wait;
			std::deque<T> m_deque;
			bool m_is_waiting;

			tsdeque(const tsdeque<T>&) = delete;

		public:
			tsdeque() : m_is_waiting(true) {
				m_deque.resize(0);
			};

			virtual ~tsdeque() {
				clear();
			}

			const T& front() {
				const std::lock_guard<std::mutex> lock(m_mutex_queue);
				return m_deque.front();
			}

			const T& back() {
				const std::lock_guard<std::mutex> lock(m_mutex_queue);
				return m_deque.back();
			}

			T pop_front() {
				const std::lock_guard<std::mutex> lock(m_mutex_queue);
				T t = std::move(m_deque.front());
				m_deque.pop_front();
				return t;
			}

			T pop_back() {
				const std::lock_guard<std::mutex> lock(m_mutex_queue);
				T t = std::move(m_deque.back());
				m_deque.pop_back();
				return t;
			}

			void push_front(const T& element) {
				const std::lock_guard<std::mutex> lock(m_mutex_queue);
				m_deque.emplace_front(std::move(element));
				std::unique_lock<std::mutex> ul(m_mutex_wait);
				m_cv_wait.notify_one();
			}

			void push_back(const T& element) {
				const std::lock_guard<std::mutex> lock(m_mutex_queue);
				m_deque.emplace_back(std::move(element));
				std::unique_lock<std::mutex> ul(m_mutex_wait);
				m_cv_wait.notify_one();
			}

			bool empty() {
				const std::lock_guard<std::mutex> lock(m_mutex_queue);
				return m_deque.empty();
			}

			size_t count() {
				const std::lock_guard<std::mutex> lock(m_mutex_queue);
				return m_deque.size();
			}

			void clear() {
				const std::lock_guard<std::mutex> lock(m_mutex_queue);
				m_deque.clear();
			}

			void wait() {
				m_is_waiting = true;
				while (empty() && m_is_waiting) {
					std::unique_lock<std::mutex> ul(m_mutex_wait);
					m_cv_wait.wait(ul);
				}
			}

			void cancel_wait() {
				m_is_waiting = false;
				m_cv_wait.notify_one();
			}
	};
}

#endif