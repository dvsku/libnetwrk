#pragma once

#include "libnetwrk/net/core/shared/shared_context.hpp"
#include "libnetwrk/net/core/service/service_connection.hpp"
#include "libnetwrk/net/core/system_commands.hpp"
#include "libnetwrk/net/enum/enums.hpp"

#include <functional>

namespace libnetwrk {
    struct service_settings {
        uint8_t gc_freq_sec       = 15U;
        uint8_t auth_deadline_sec = 10U;
    };

    template<typename Connection>
    class service_context : public shared_context<Connection> {
    public:
        using connection_t = typename Connection::base_t;

        using cb_start_t          = std::function<void()>;
        using cb_stop_t           = std::function<void()>;
        using cb_before_connect_t = std::function<bool(std::shared_ptr<connection_t>)>;
        using cb_disconnect_t     = std::function<void(std::shared_ptr<connection_t>, libnetwrk::disconnect_code)>;

    public:
        service_settings settings;

        cb_start_t          cb_start;
        cb_stop_t           cb_stop;
        cb_before_connect_t cb_before_connect;
        cb_disconnect_t     cb_disconnect;

    public:
        service_context()
            : shared_context<Connection>() {}
    };
}
