#pragma once

#include "libnetwrk/net/core/base_connection.hpp"

namespace libnetwrk {
    /*
        Base connection that represents a service's client connection
    */
    template<typename Desc, typename Socket>
    class base_client_connection : public base_connection<Desc, Socket> {
    public:
        using base_connection_t = base_connection<Desc, Socket>;
        using socket_t          = Socket;
        using context_t         = base_connection_t::context_t;
        using storage_t         = base_connection_t::storage_t;

    public:
        base_client_connection()                              = delete;
        base_client_connection(const base_client_connection&) = delete;
        base_client_connection(base_client_connection&&)      = default;

        base_client_connection(context_t& context, socket_t socket)
            : base_connection_t(context, std::move(socket)) {}

        base_client_connection& operator=(const base_client_connection&) = delete;
        base_client_connection& operator=(base_client_connection&&)      = default;

    public:
        /*
            Get connection storage
        */
        storage_t& get_storage() override {
            return this->m_storage;
        }
    };
}
