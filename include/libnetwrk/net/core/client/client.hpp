#pragma once

#include "libnetwrk/net/core/client/client_context.hpp"
#include "libnetwrk/net/core/client/client_comp_connection.hpp"
#include "libnetwrk/net/core/client/client_comp_message.hpp"
#include "libnetwrk/net/core/client/client_comp_system_message.hpp"
#include "libnetwrk/net/core/client/client_connection_internal.hpp"

#include <string>
#include <cstdint>

namespace libnetwrk {
    template<typename Desc, typename Socket>
    class client {
    public:
        using context_t             = client_context<client_connection_internal<Desc, Socket>>;
        using comp_connection_t     = client_comp_connection<context_t>;
        using comp_message_t        = client_comp_message<context_t>;
        using comp_system_message_t = client_comp_system_message<context_t>;

        using client_t        = client<Desc, Socket>;
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
              m_comp_system_message(m_context, m_comp_message)
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

        /*
            Adjust a service timestamp to account for clock drift
        */
        const void adjust_timestamp(uint64_t timestamp) const {
            timestamp -= m_context.clock_drift;
        }

        /*
            Set message callback

            @param void(command_t, owned_message_t*) func
        */
        void set_message_callback(context_t::cb_message_t cb) {
            if (!m_context.cb_message)
                m_context.cb_message = cb;
        }

        /*
            Set connected callback

            @param void(std::shared_ptr<connection_t>) func
        */
        void set_connect_callback(context_t::cb_connect_t cb) {
            if (!m_context.cb_connect)
                m_context.cb_connect = cb;
        }

        /*
            Set disconnected callback

            @param void() func
        */
        void set_disconnect_callback(context_t::cb_disconnect_t cb) {
            if (!m_context.cb_disconnect)
                m_context.cb_disconnect = cb;
        }

        /*
            Set pre process message data callback

            @param void(dynamic_buffer*) func
        */
        void set_pre_process_message_callback(context_t::cb_pre_process_message_t cb) {
            if (!m_context.cb_pre_process_message)
                m_context.cb_pre_process_message = cb;
        }

        /*
            Set post process message data callback

            @param void(dynamic_buffer*) func
        */
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
            m_context.cancel_cv.notify_all();
            m_context.cancel_cv.wait_for_end();

            m_comp_connection.stop_connection();

            m_context.stop_io_context();
            m_comp_connection.connection.reset();

            m_comp_message.stop_processing_messages();
        }

        virtual bool connect_impl(const std::string& host, const uint16_t port) {
            return false;
        }
    };
}
