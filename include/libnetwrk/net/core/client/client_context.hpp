#pragma once

#include "libnetwrk/net/core/libnetwrk_context.hpp"

namespace libnetwrk {
    template<typename tn_connection>
    class client_context : public libnetwrk_context<tn_connection> {
    public:
        using cb_disconnect_t = std::function<void()>;

    public:
        cb_disconnect_t cb_disconnect;  
    };
}
