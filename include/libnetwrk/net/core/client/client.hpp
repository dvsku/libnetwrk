#pragma once

#include "libnetwrk/net/core/client/client_mngr_system_messages.hpp"

#include <string>
#include <cstdint>

namespace libnetwrk {
    template<typename tn_desc, typename tn_socket>
    class client : public client_mngr_system_messages<tn_desc, tn_socket> {
    public:
        using client_t        = client<tn_desc, tn_socket>;
        using connection_t    = client_mngr_system_messages<tn_desc, tn_socket>::connection_t;
        using message_t       = connection_t::message_t;
        using owned_message_t = connection_t::owned_message_t;

    public:
        client()                = delete;
        client(const client_t&) = delete;
        client(client_t&&)      = default;

        client(const std::string& name)
            : client_mngr_system_messages<tn_desc, tn_socket>()
        {
            this->m_name = name;

            this->set_internal_disconnect_callback([this] {
                std::thread t = std::thread([this] {
                    this->disconnect();
                });
                t.detach();
            });
        }

        client_t& operator=(const client_t&) = delete;
        client_t& operator=(client_t&&)      = default;

    public:
        /*
            Disconnect the client and clean up.
        */
        void disconnect() {
            if (this->m_status != service_status::started)
                return;

            this->m_status = service_status::stopping;
            this->teardown();

            if (this->m_ev_disconnect_callback)
                this->m_ev_disconnect_callback();

            LIBNETWRK_INFO(this->m_name, "Disconnected.");

            this->m_status = service_status::stopped;
        }

        /*
            Connect to service.
        */
        bool connect(const std::string& host, const uint16_t port) {
            if (this->m_status != service_status::stopped)
                return false;

            this->m_status = service_status::starting;

            bool connected = connect_impl(host, port);

            if (connected) {
                if (this->m_ev_connect_callback)
                    this->m_ev_connect_callback();

                this->m_status = service_status::started;
            }
            else {
                this->m_status = service_status::stopped;
            }

            return connected;
        }

    protected:
        virtual void teardown() {
            if (this->m_connection)
                this->m_connection->stop();

            /*
                Wait for all coroutines to stop
            */
            wait_for_coroutines_to_stop();

            this->stop_io_context();
            this->m_connection.reset();

            this->stop_processing_messages();
        }

        virtual bool connect_impl(const std::string& host, const uint16_t port) {
            return false;
        }

    private:
        void wait_for_coroutines_to_stop() {
            while (true) {
                if (!this->m_connection)                        break;
                if (this->m_connection->active_operations == 0) break;

                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }
    };
}
