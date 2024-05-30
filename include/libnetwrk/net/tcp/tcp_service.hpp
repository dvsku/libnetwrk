#pragma once

#include "libnetwrk/net/default_service_desc.hpp"
#include "libnetwrk/net/tcp/socket.hpp"
#include "libnetwrk/net/tcp/tcp_resolver.hpp"
#include "libnetwrk/net/core/service/service.hpp"

#include <exception>
#include <thread>

namespace libnetwrk::tcp {
    template<typename tn_desc = libnetwrk::default_service_desc>
    requires is_libnetwrk_service_desc<tn_desc>
    class tcp_service : public libnetwrk::service<tn_desc, libnetwrk::tcp::socket> {
    public:
        using base_t                = libnetwrk::service<tn_desc, libnetwrk::tcp::socket>;
        using connection_t          = base_t::context_t::connection_t;
        using connection_internal_t = base_t::context_t::connection_internal_t;
        using command_t             = typename tn_desc::command_t;
        using message_t             = base_t::message_t;
        using owned_message_t       = base_t::owned_message_t;

    public:
        tcp_service(const std::string& name = "TCP service")
            : base_t(name), m_acceptor(this->m_context.io_context) {};

        virtual ~tcp_service() {
            this->m_context.status = service_status::stopping;
            this->teardown();
        }

        uint16_t get_port() {
            return m_acceptor.local_endpoint().port();
        }

    protected:
        using acceptor_t = asio::ip::tcp::acceptor;

    protected:
        acceptor_t m_acceptor;

    private:
        void teardown() override final {
            if (m_acceptor.is_open())
                m_acceptor.close();

            base_t::teardown();
        };

        bool start_impl(const std::string& host, const uint16_t port) override final {
            using namespace asio::experimental::awaitable_operators;

            try {
                // Create resolver
                tcp_resolver resolver(this->m_context.io_context);

                // Resolve hostname
                asio::ip::tcp::endpoint ep;
                if (!resolver.get_endpoint(host, port, ep))
                    throw libnetwrk_exception("Failed to resolve hostname.");

                // Open acceptor
                m_acceptor.open(asio::ip::tcp::v4());
                m_acceptor.set_option(acceptor_t::reuse_address(true));
                m_acceptor.bind(ep);
                m_acceptor.listen();

                // Start listening
                asio::co_spawn(this->m_context.io_context, co_listen() || this->m_context.cancel_cv.wait(), [this](auto, auto) {
                    LIBNETWRK_INFO(this->m_context.name, "Stopped listening.");
                });

                // Start GC
                this->m_comp_connection.start_gc();

                // Start context
                this->m_context.start_io_context();
            }
            catch (const std::exception& e) {
                (void)e;

                LIBNETWRK_ERROR(this->m_context.name, "Failed to start listening. | {}", e.what());
                this->teardown();
                return false;
            }
            catch (...) {
                LIBNETWRK_ERROR(this->m_context.name, "Failed to start listening. | Critical fail.");
                this->teardown();
                return false;
            }

            return true;
        }

    private:
        asio::awaitable<void> co_listen() {
            auto current_executor = co_await asio::this_coro::executor;

            LIBNETWRK_INFO(this->m_context.name, "Listening for connections on {}:{}.",
                m_acceptor.local_endpoint().address().to_string(),
                m_acceptor.local_endpoint().port());

            while (true) {
                auto connection = std::make_shared<connection_internal_t>(this->m_context.io_context);

                auto [ec] = co_await m_acceptor.async_accept(connection->get_socket().native(),
                    asio::as_tuple(asio::use_awaitable));

                if (ec) {
                    if (ec != asio::error::operation_aborted) {
                        LIBNETWRK_ERROR(this->m_context.name, "Failed to accept connection. | {}: {}", ec.value(), ec.message());
                    }

                    break;
                }

                asio::co_spawn(current_executor, co_accept(connection), asio::detached);
            }
        }

        asio::awaitable<void> co_accept(std::shared_ptr<connection_internal_t> connection) {
            LIBNETWRK_VERBOSE(this->m_context.name, "Attempted connection from {}:{}.",
                connection->get_ip(), connection->get_port());

            if (!this->m_context.cb_before_connect || this->m_context.cb_before_connect(std::static_pointer_cast<connection_t>(connection))) {
                this->m_comp_connection.accept_connection(connection);
                connection->set_id(++this->m_comp_connection.id_counter);
                this->m_comp_message.start_connection_read_and_write(connection);
                this->m_comp_system_message.send_auth_message(connection);

                if (this->m_context.cb_connect)
                    this->m_context.cb_connect(std::static_pointer_cast<connection_t>(connection));

                LIBNETWRK_INFO(this->m_context.name, "Connection success from {}:{}.",
                    connection->get_ip(), connection->get_port());
            }
            else {
                LIBNETWRK_WARNING(this->m_context.name, "Connection denied.");
            }

            co_return;
        }
    };
}
