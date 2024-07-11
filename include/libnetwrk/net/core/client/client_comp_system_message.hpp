#pragma once

#include "asio.hpp"
#include "asio/experimental/awaitable_operators.hpp"
#include "libnetwrk/net/misc/authentication.hpp"
#include "libnetwrk/net/core/client/client_comp_message.hpp"
#include "libnetwrk/net/misc/coroutine_cv.hpp"

namespace libnetwrk {
    template<typename Context>
    class client_comp_system_message {
    public:
        using context_t             = Context;
        using comp_message_t        = client_comp_message<Context>;
        using connection_internal_t = context_t::connection_internal_t;
        using message_t             = context_t::message_t;
        using owned_message_t       = context_t::owned_message_t;

    public:
        client_comp_system_message(context_t& context, comp_message_t& comp_message)
            : m_context(context), m_comp_message(comp_message)
        {
            m_context.cb_system_message = [this](auto command, auto message) {
                ev_system_message(command, message);
            };
        }

    public:
        void send_clock_sync_request(uint8_t sample_index) {
            message_t message{};
            message.head.type    = message_type::system;
            message.head.command = static_cast<uint64_t>(system_command::cev_clock_sync);
            message << sample_index << get_milliseconds_timestamp();

            m_comp_message.send(message, libnetwrk::send_flags::none);
        }

    private:
        context_t&      m_context;
        comp_message_t& m_comp_message;

    private:

    private:
        void start_clock_syncing() {
            using namespace asio::experimental::awaitable_operators;

            asio::co_spawn(m_context.io_context, co_clock_sync() || m_context.cancel_cv.wait(), [this](auto, auto) {
                LIBNETWRK_DEBUG(m_context.name, "Stopped clock syncing.");
            });
        }

        asio::awaitable<void> co_clock_sync() {
            asio::steady_timer timer(m_context.io_context, std::chrono::seconds(m_context.settings.clock_sync_freq_sec));

            while (true) {
                send_clock_sync_request(0U);

                timer.expires_after(std::chrono::seconds(m_context.settings.clock_sync_freq_sec));

                auto [ec] = co_await timer.async_wait(asio::as_tuple(asio::use_awaitable));
                if (ec) break;
            }
        }

    private:
        void ev_system_message(system_command command, owned_message_t* message) {
            switch (command) {
                case system_command::s2c_verify:     return on_system_verify_message(message);
                case system_command::s2c_verify_ok:  return on_system_verify_ok_message(message);
                case system_command::sev_clock_sync: return on_system_clock_sync_message(message);
                
                default: return;
            }
        }

        void on_system_verify_message(owned_message_t* message) {
            authentication::request_t  auth_request{};
            authentication::response_t auth_response{};

            message->message >> auth_request;
            auth_response = authentication::generate_response(auth_request);

            message_t response{};
            response.head.type    = message_type::system;
            response.head.command = static_cast<uint64_t>(system_command::c2s_verify);
            response << auth_response;

            message->sender->send(response);
        }

        void on_system_verify_ok_message(owned_message_t* msg) {
            auto connection = std::static_pointer_cast<connection_internal_t>(msg->sender);
            connection->is_authenticated = true;
            connection->write_cv.notify_all();

            start_clock_syncing();
        }

        void on_system_clock_sync_message(owned_message_t* message) {
            uint8_t  sample_index      = 0U;
            uint64_t client_timestamp  = 0U;
            uint64_t service_timestamp = 0U;
            uint64_t current_timestamp = get_milliseconds_timestamp();

            message->message >> sample_index >> client_timestamp >> service_timestamp;

            if (sample_index < m_context.clock_drift_samples.size()) {
                /*
                    Sample is equal to the difference in current time and sent service time
                    plus the end to end latency.
                */

                m_context.clock_drift_samples[sample_index]
                    = static_cast<int32_t>(current_timestamp - service_timestamp + (current_timestamp - client_timestamp) / 2);

                m_context.clock_drift_samples_received++;
            }        

            /*
                If not the final sample, continue sampling.
            */

            if (m_context.clock_drift_samples_received != m_context.clock_drift_samples.size())
                return send_clock_sync_request(++sample_index);
            
            /*
                Discard retransmitted samples.
            */

            std::sort(m_context.clock_drift_samples.begin(), m_context.clock_drift_samples.end());

            size_t max_index = m_context.clock_drift_samples.size() - 1;

            for (size_t i = max_index; i > 0; i--) {
                if (m_context.clock_drift_samples[i] < m_context.clock_drift_samples[0] * 1.55f) {
                    max_index = i;
                    break;
                }
            }

            /*
                Calculate clock drift
            */

            int64_t accumulated = 0;
            for (size_t i = 0; i <= max_index; i++) {
                accumulated += m_context.clock_drift_samples[i];
            }

            m_context.clock_drift = static_cast<int32_t>(std::round(accumulated / (max_index + 1)));
            
            m_context.clock_drift_samples_received = 0U;
            m_context.clock_drift_samples          = {};

            LIBNETWRK_DEBUG(m_context.name, "Clock synced.");
        }
    };
}
