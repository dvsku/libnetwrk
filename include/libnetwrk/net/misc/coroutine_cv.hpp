#pragma once

#include "asio.hpp"

#include <atomic>

namespace libnetwrk {
    /*
        Condition variable for coroutines.
    */
    class coroutine_cv {
    public:
        coroutine_cv()                    = delete;
        coroutine_cv(const coroutine_cv&) = delete;
        coroutine_cv(coroutine_cv&&)      = default;

        coroutine_cv(asio::io_context& context)
            : m_io_context(context), m_timer(context, asio::steady_timer::duration::max())
        {}

        coroutine_cv& operator=(const coroutine_cv&) = delete;
        coroutine_cv& operator=(coroutine_cv&&)      = default;

    public:
        /*
            Wait for notify or expire.
        */
        asio::awaitable<void> wait() {
            m_operations++;
            co_await m_timer.async_wait(asio::as_tuple(asio::use_awaitable));
            m_operations--;
            co_return;
        }

        /*
            Wait for all operations to finish.
        */
        void wait_for_end() {
            while (true) {
                if (m_operations == 0 || m_io_context.stopped())
                    break;

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        bool has_active_operations() {
            return m_operations != 0;
        }

        void notify_one() {
            m_timer.cancel_one();
        }

        void notify_all() {
            m_timer.cancel();
        }

    private:
        asio::io_context&    m_io_context;
        asio::steady_timer   m_timer;
        std::atomic_uint16_t m_operations = 0U;

    private:
        asio::awaitable<void> co_wait_for_end() {
            asio::system_timer timer{ co_await asio::this_coro::executor };

            while (true) {
                timer.expires_after(std::chrono::milliseconds{ 5 });
                auto [ec] = co_await timer.async_wait(asio::as_tuple(asio::use_awaitable));

                if (ec || m_operations == 0)
                    break;
            }

            co_return;
        }
    };
}
