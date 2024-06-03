#pragma once

#include "asio.hpp"
#include "asio/experimental/awaitable_operators.hpp"
#include "libnetwrk/net/misc/coroutine_cv.hpp"

#include <list>
#include <mutex>

namespace libnetwrk {
    template<typename Context>
    class service_comp_connection {
    public:
        using context_t    = Context;
        using connection_t = typename Context::connection_internal_t;

    public:
        std::list<std::shared_ptr<connection_t>> connections;
        std::mutex                               connections_mutex;
        uint64_t                                 id_counter = 0U;

    public:
        service_comp_connection(context_t& context)
            : m_context(context) {}

    public:
        std::shared_ptr<connection_t> create_connection() {
            return std::make_shared<connection_t>(m_context.io_context);
        }

        void accept_connection(std::shared_ptr<connection_t> connection) {
            std::lock_guard<std::mutex> guard(connections_mutex);
            connections.push_back(connection);
        }

        void start_gc() {
            using namespace asio::experimental::awaitable_operators;

            asio::co_spawn(m_context.io_context, co_gc() || m_context.cancel_cv.wait(), [this](auto, auto) {
                LIBNETWRK_VERBOSE(m_context.name, "Stopped GC.");
            });
        }

    private:
        context_t& m_context;

    private:
        asio::awaitable<void> co_gc() {
            asio::steady_timer timer(m_context.io_context, std::chrono::seconds(m_context.settings.gc_freq_sec));
            
            size_t count_before = 0U;
            size_t count_after  = 0U;

            while (true) {
                auto [ec] = co_await timer.async_wait(asio::as_tuple(asio::use_awaitable));

                if (ec)
                    break;

                {
                    std::lock_guard<std::mutex> guard(connections_mutex);

                    count_before = connections.size();

                    connections.remove_if([this](auto& client) {
                        if (!client)
                            return true;

                        if (!client->is_connected() && !client->cancel_cv.has_active_operations()) {
                            if (m_context.cb_disconnect)
                                m_context.cb_disconnect(client, client->disconnect_code);

                            return true;
                        }

                        if (!client->is_authenticated) {
                            uint64_t timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

                            if (timestamp > client->auth_request_deadline) {
                                client->disconnect_code = libnetwrk::disconnect_code::authentication_failed;

                                if (m_context.cb_internal_disconnect)
                                    m_context.cb_internal_disconnect(client);
                            }
                        }

                        return false;
                    });

                    count_after = connections.size();
                }

                LIBNETWRK_VERBOSE(m_context.name, "GC tc: {} rc: {}", count_after, count_before - count_after);
                timer.expires_after(std::chrono::seconds(m_context.settings.gc_freq_sec));
            }
        }
    };
}