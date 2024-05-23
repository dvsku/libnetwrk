#pragma once

#include "asio.hpp"
#include "libnetwrk/net/core/context.hpp"

namespace libnetwrk {
    /*
        Condition variable for coroutines.
    */
    class coroutine_cv {
    public:
        coroutine_cv()                    = delete;
        coroutine_cv(const coroutine_cv&) = delete;
        coroutine_cv(coroutine_cv&&)      = default;

        coroutine_cv(work_context& context)
            : m_timer(*context.io_context, asio::steady_timer::time_point::max())
        {}

        coroutine_cv& operator=(const coroutine_cv&) = delete;
        coroutine_cv& operator=(coroutine_cv&&)      = default;

    public:
        asio::awaitable<void> wait() {
            co_await m_timer.async_wait(asio::as_tuple(asio::use_awaitable));
            co_return;
        }

        void notify_one() {
            m_timer.cancel_one();
        }

        void notify_all() {
            m_timer.cancel();
        }

    private:
        asio::steady_timer m_timer;
    };
}
