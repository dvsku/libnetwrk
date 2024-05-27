#pragma once

#include "asio.hpp"
#include "libnetwrk/net/core/client/client_connection.hpp"
#include "libnetwrk/net/core/system_commands.hpp"
#include "libnetwrk/net/core/enums.hpp"

#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include <functional>

namespace libnetwrk {
    template<typename tn_connection>
    class libnetwrk_context {
    public:
        using io_context_t = asio::io_context;

        using connection_internal_t = tn_connection;
        using connection_t          = connection_internal_t::base_t;
        using command_t             = connection_t::command_t;
        using endpoint_t            = connection_internal_t::endpoint_t;
        using message_t             = connection_t::message_t;
        using owned_message_t       = connection_t::owned_message_t;
        using outgoing_message_t    = connection_t::outgoing_message_t;
        using buffer_t              = message_t::buffer_t;

        using cb_message_t              = std::function<void(command_t,      owned_message_t*)>;
        using cb_system_message_t       = std::function<void(system_command, owned_message_t*)>;  
        using cb_connect_t              = std::function<void(std::shared_ptr<connection_t>)>;
        using cb_internal_disconnect_t  = std::function<void(std::shared_ptr<connection_internal_t>)>;
        using cb_pre_process_message_t  = std::function<void(buffer_t*)>;
        using cb_post_process_message_t = std::function<void(buffer_t*)>;

    public:
        libnetwrk_context()
            : io_context(1) {}

    public:
        std::string         name   = "";
        std::atomic_uint8_t status = libnetwrk::service_status::stopped;
        io_context_t        io_context;

        cb_message_t              cb_message;
        cb_system_message_t       cb_system_message;
        cb_connect_t              cb_connect;
        cb_internal_disconnect_t  cb_internal_disconnect;
        cb_pre_process_message_t  cb_pre_process_message;
        cb_post_process_message_t cb_post_process_message;

    public:
        bool is_running() const {
            return status == service_status::started;
        }

        void start_io_context() {
            m_io_context_thread = std::thread([this] { 
                this->io_context.run(); 
            });
        }

        void stop_io_context() {
            if (!io_context.stopped())
                io_context.stop();

            if(m_io_context_thread.joinable())
               m_io_context_thread.join();
        }

    private:
        std::thread m_io_context_thread;
    };
}
