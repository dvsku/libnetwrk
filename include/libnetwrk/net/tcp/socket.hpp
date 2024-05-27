#pragma once

#include "asio.hpp"
#include "libnetwrk/net/core/containers/buffer.hpp"

#include <cstdint>
#include <cstddef>
#include <system_error>
#include <functional>

namespace libnetwrk::tcp {
    class socket {
    public:
        using native_socket_t  = asio::ip::tcp::socket;
        using io_context_t     = asio::io_context;
        using endpoint_t       = asio::ip::tcp::endpoint;
        using read_callback_t  = std::function<void(std::error_code, std::size_t)>;
        using write_callback_t = std::function<void(std::error_code, std::size_t)>;

    public:
        socket(io_context_t& context)
            : m_socket(context) {}

        socket(native_socket_t socket)
            : m_socket(std::move(socket)) {}

    public:
        /*
            Get socket ip address.
        */
        std::string get_ip() {
            return m_socket.remote_endpoint().address().to_string();
        }

        /*
            Get socket port.
        */
        uint16_t get_port() {
            return m_socket.remote_endpoint().port();
        }

        /*
            Check if socket is connected.
        */
        bool is_connected() {
            return m_socket.is_open();
        }

        void connect(const endpoint_t& endpoint) {
            m_socket.connect(endpoint);
        }

        bool connect(const endpoint_t& endpoint, std::error_code& ec) {
            m_socket.connect(endpoint, ec);
            return !ec;
        }

    public:
        native_socket_t& native() {
            return m_socket;
        }

        void close() {
            if (m_socket.is_open())
                m_socket.close();
        }

        template<typename Serialize>
        asio::awaitable<std::tuple<std::error_code, size_t>> async_read(libnetwrk::buffer<Serialize>& buffer) {
            std::tuple<std::error_code, size_t> result = co_await asio::async_read(m_socket,
                asio::buffer(buffer.data(), buffer.size()), asio::as_tuple(asio::use_awaitable));

            co_return result;
        }

        template<typename Serialize>
        asio::awaitable<std::tuple<std::error_code, size_t>> async_write(libnetwrk::buffer<Serialize>& buffer) {
            std::tuple<std::error_code, size_t> result = co_await asio::async_write(m_socket,
                asio::buffer(buffer.data(), buffer.size()), asio::as_tuple(asio::use_awaitable));

            co_return result;
        }

    private:
        native_socket_t m_socket;
    };
}
