#pragma once

#include "libnetwrk/net/core/client/client_context.hpp"
#include "libnetwrk/net/core/client/client_comp_connection.hpp"
#include "libnetwrk/net/core/client/client_comp_message.hpp"
#include "libnetwrk/net/core/client/client_comp_system_message.hpp"
#include "libnetwrk/net/core/client/client_connection_internal.hpp"

#include <string>
#include <cstdint>

namespace libnetwrk {
    template<typename tn_desc, typename tn_socket>
    class client {
    public:
        using context_t             = client_context<client_connection_internal<tn_desc, tn_socket>>;
        using comp_connection_t     = client_comp_connection<context_t>;
        using comp_message_t        = client_comp_message<context_t>;
        using comp_system_message_t = client_comp_system_message<context_t>;

        using client_t        = client<tn_desc, tn_socket>;
        using connection_t    = context_t::connection_t;
        using message_t       = context_t::message_t;
        using owned_message_t = context_t::owned_message_t;

    public:
        client()                = delete;
        client(const client_t&) = delete;
        client(client_t&&)      = default;

        client(const std::string& name)
            : m_comp_connection(m_context),
              m_comp_message(m_context, m_comp_connection),
              m_comp_system_message(m_context)
        {
            m_context.name = name;

            m_context.cb_internal_disconnect = [this](auto) {
                std::thread t = std::thread([this] {
                    this->disconnect();
                });
                t.detach();
            };
        }

        client_t& operator=(const client_t&) = delete;
        client_t& operator=(client_t&&)      = default;

    public:
        bool is_connected() {
            return m_context.is_running();
        }

        /*
            Connect to service.
        */
        bool connect(const std::string& host, const uint16_t port) {
            if (m_context.status != to_underlying(service_status::stopped))
                return false;

            m_context.status = to_underlying(service_status::starting);

            bool connected = connect_impl(host, port);

            if (connected) {
                if (m_context.cb_connect)
                    m_context.cb_connect(m_comp_connection.connection);

                m_context.status = to_underlying(service_status::started);
            }
            else {
                m_context.status = to_underlying(service_status::stopped);
            }

            return connected;
        }

        /*
            Disconnect the client and clean up.
        */
        void disconnect() {
            if (m_context.status != to_underlying(service_status::started))
                return;

            m_context.status = to_underlying(service_status::stopping);
            this->teardown();

            if (m_context.cb_disconnect)
                m_context.cb_disconnect();

            LIBNETWRK_INFO(m_context.name, "Disconnected.");

            m_context.status = to_underlying(service_status::stopped);
        }

        void send(message_t& message, libnetwrk::send_flags flags = libnetwrk::send_flags::none) {
            m_comp_message.send(message, flags);
        }

        bool process_message() {
            return m_comp_message.process_message();
        }

        bool process_messages() {
            return m_comp_message.process_messages();
        }

        bool process_messages_async() {
            return m_comp_message.process_messages_async();
        }

    public:
        const std::string& get_name() const {
            return m_context.name;
        }

        void set_message_callback(context_t::cb_message_t cb) {
            if (!m_context.cb_message)
                m_context.cb_message = cb;
        }

        void set_pre_process_message_callback(context_t::cb_pre_process_message_t cb) {
            if (!m_context.cb_pre_process_message)
                m_context.cb_pre_process_message = cb;
        }

        void set_post_process_message_callback(context_t::cb_post_process_message_t cb) {
            if (!m_context.cb_post_process_message)
                m_context.cb_post_process_message = cb;
        }

    protected:
        context_t             m_context;
        comp_connection_t     m_comp_connection;
        comp_message_t        m_comp_message;
        comp_system_message_t m_comp_system_message;

    protected:
        virtual void teardown() {
            if (m_comp_connection.connection)
                m_comp_connection.connection->stop();

            /*
                Wait for all coroutines to stop
            */
            wait_for_coroutines_to_stop();

            m_context.stop_io_context();
            m_comp_connection.connection.reset();

            m_comp_message.stop_processing_messages();
        }

        virtual bool connect_impl(const std::string& host, const uint16_t port) {
            return false;
        }

    private:
        void wait_for_coroutines_to_stop() {
            if (!m_comp_connection.connection)                                    return;
            if (!m_comp_connection.connection->cancel_cv.has_active_operations()) return;

            m_comp_connection.connection->cancel_cv.wait_for_end();
        }
    };
}
