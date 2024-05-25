#pragma once

#include "asio.hpp"
#include "libnetwrk/net/core/enums.hpp"

#include <string>
#include <memory>
#include <atomic>
#include <thread>

namespace libnetwrk {
    class libnetwrk_context {
    protected:
        using io_context_t = asio::io_context;

    protected:
        std::string                   m_name   = "";
        std::atomic_uint8_t           m_status = libnetwrk::service_status::stopped;
        std::unique_ptr<io_context_t> m_io_context;

    protected:
        bool is_running() {
            return m_status == service_status::started;
        }

        void create_io_context() {
            m_io_context = std::make_unique<io_context_t>(1);
        }

        void start_io_context() {
            m_io_context_thread = std::thread([this] { this->m_io_context->run(); });
        }

        void stop_io_context() {
            if (m_io_context && !m_io_context->stopped())
                m_io_context->stop();

            if (m_io_context_thread.joinable())
                m_io_context_thread.join();
        }

    private:
        std::thread m_io_context_thread;
    };
}
