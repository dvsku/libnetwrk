#pragma once

#include <deque>
#include <mutex>

namespace libnetwrk {
    template<typename T> 
    class tsdeque {
    public:
        using tsdeque_t   = tsdeque<T>;
        using lock_guard  = std::lock_guard<std::mutex>;
        using unique_lock = std::unique_lock<std::mutex>;

    public:
        tsdeque() {
            m_deque.resize(0);
        };

        tsdeque(const tsdeque_t&) = delete;
        tsdeque(tsdeque_t&&)      = default;

        virtual ~tsdeque() {
            clear();
        }

        tsdeque_t& operator=(const tsdeque_t&) = delete;
        tsdeque_t& operator=(tsdeque_t&&)      = default;

    public:
        const T& front() {
            const lock_guard lock(m_mutex_queue);
            return m_deque.front();
        }

        const T& back() {
            const lock_guard lock(m_mutex_queue);
            return m_deque.back();
        }

        T pop_front() {
            const lock_guard lock(m_mutex_queue);
            T t = std::move(m_deque.front());
            m_deque.pop_front();
            return t;
        }

        T pop_back() {
            const lock_guard lock(m_mutex_queue);
            T t = std::move(m_deque.back());
            m_deque.pop_back();
            return t;
        }

        void push_front(const T& element) {
            const lock_guard lock(m_mutex_queue);
            m_deque.emplace_front(std::move(element));
            unique_lock ul(m_mutex_wait);
            m_cv_wait.notify_one();
        }

        void push_back(const T& element) {
            const lock_guard lock(m_mutex_queue);
            m_deque.emplace_back(std::move(element));
            unique_lock ul(m_mutex_wait);
            m_cv_wait.notify_one();
        }

        bool empty() {
            const lock_guard lock(m_mutex_queue);
            return m_deque.empty();
        }

        size_t count() {
            const lock_guard lock(m_mutex_queue);
            return m_deque.size();
        }

        void clear() {
            const lock_guard lock(m_mutex_queue);
            m_deque.clear();
        }

        void wait() {
            m_is_waiting = true;
            while (empty() && m_is_waiting) {
                unique_lock ul(m_mutex_wait);
                m_cv_wait.wait(ul);
            }
        }

        void cancel_wait() {
            m_is_waiting = false;
            m_cv_wait.notify_one();
        }

    protected:
        std::mutex              m_mutex_queue;
        std::mutex              m_mutex_wait;
        std::condition_variable m_cv_wait;
        std::deque<T>           m_deque;
        
        bool m_is_waiting = true;

    };
}
