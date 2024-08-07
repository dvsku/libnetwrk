#pragma once

#include "libnetwrk/net/core/shared/shared_context.hpp"

#include <array>

namespace libnetwrk {
    struct client_settings {
        uint16_t clock_sync_freq_sec = 120U;
    };

    template<typename Connection>
    class client_context : public shared_context<Connection> {
    public:
        using cb_disconnect_t = std::function<void(bool)>;

    public:
        client_settings settings;

        uint8_t                 clock_drift_samples_received = 0U;
        std::array<int32_t, 10> clock_drift_samples          = {};

        cb_disconnect_t cb_disconnect;

    public:
        client_context()
            : shared_context<Connection>() {}
    };
}
