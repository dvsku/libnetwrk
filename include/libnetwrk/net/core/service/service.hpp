#pragma once

#include "libnetwrk/net/core/service/service_context.hpp"
#include "libnetwrk/net/core/service/service_comp_connection.hpp"
#include "libnetwrk/net/core/service/service_comp_message.hpp"
#include "libnetwrk/net/core/service/service_comp_system_message.hpp"
#include "libnetwrk/net/core/service/service_connection_internal.hpp"

#include <algorithm>

namespace libnetwrk {
    template<typename tn_desc, typename tn_socket>
    class service {
    public:
        using context_t             = service_context<service_connection_internal<tn_desc, tn_socket>>;
        using comp_connection_t     = service_comp_connection<context_t>;
        using comp_message_t        = service_comp_message<context_t>;
        using comp_system_message_t = service_comp_system_message<context_t>;

        using service_t       = service<tn_desc, tn_socket>;
        using connection_t    = context_t::connection_t;
        using message_t       = context_t::message_t;
        using owned_message_t = context_t::owned_message_t;

    public:
        service()                 = delete;
        service(const service_t&) = delete;
        service(service_t&&)      = default;

        service(const std::string& name)
            : m_comp_connection(m_context),
              m_comp_message(m_context, m_comp_connection),
              m_comp_system_message(m_context)
        {
            m_context.name = name;

            m_context.cb_internal_disconnect = [this](auto connection) {
                connection->stop();
                //std::thread t = std::thread([this] {
                //    this->disconnect();
                //});
                //t.detach();
            };
        }

        service_t& operator=(const service_t&) = delete;
        service_t& operator=(service_t&&)      = default;

    public:
        bool is_running() {
            return m_context.is_running();
        }

        bool start(const std::string& host, const uint16_t port) {
            if (m_context.status != service_status::stopped)
                return false;

            m_context.status = service_status::starting;

            bool started = start_impl(host, port);

            if (started) {
                if (m_context.cb_start)
                    m_context.cb_start();

                m_context.status = service_status::started;
            }
            else {
                m_context.status = service_status::stopped;
            }

            return started;
        }

        void stop() {
            if (m_context.status != service_status::started)
                return;

            m_context.status = service_status::stopping;

            this->teardown();

            m_context.status = service_status::stopped;

            if (m_context.cb_stop)
                m_context.cb_stop();
        }

        void send(std::shared_ptr<connection_t> client, message_t& message, libnetwrk::send_flags flags = libnetwrk::send_flags::none) {
            m_comp_message.send(client, message, flags);
        }

        void send_all(message_t& message, libnetwrk::send_flags flags = libnetwrk::send_flags::none,
            comp_message_t::send_predicate_t predicate = nullptr)
        {
            m_comp_message.send_all(message, flags, predicate);
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

        service_settings& get_settings() {
            return m_context.settings;
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

        void set_connect_callback(context_t::cb_connect_t cb) {
            if (!m_context.cb_connect)
                m_context.cb_connect = cb;
        }

        void set_disconnect_callback(context_t::cb_disconnect_t cb) {
            if (!m_context.cb_disconnect)
                m_context.cb_disconnect = cb;
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

            stop_all_connections();

            m_context.stop_io_context();
            m_comp_message.stop_processing_messages();
        }

        virtual bool start_impl(const std::string& host, const uint16_t port) {
            return false;
        }

    private:
        void stop_all_connections() {
            std::lock_guard<std::mutex> guard(m_comp_connection.connections_mutex);

            for (auto& client : m_comp_connection.connections) {
                if (!client) continue;

                client->stop();

                if (client->cancel_cv.has_active_operations())
                    client->cancel_cv.wait_for_end();
            }
        }
    };
}